/*
 * NAME: Shrea Chari
 * EMAIL: shreachari@gmail.com
 * ID: 005318456
 * NAME: Dhakshin Suriakannu
 * EMAIL: dhakshin.s@g.ucla.edu
 * ID: 605280083
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include "ext2_fs.h"
#include <time.h>

unsigned int block_size = 0;

void print_err(int x, char *err)
{
    if (x < 0)
    {
        fprintf(stderr, "%s", err);
        exit(1);
    }
}

int calc_block_offset(int block)
{
    return (block * block_size);
}

char get_filetype(int i_mode)
{
    if (S_ISDIR(i_mode))
    {
        return 'd';
    }
    else if (S_ISREG(i_mode))
    {
        return 'f';
    }
    else if (S_ISLNK(i_mode))
    {
        return 's';
    }
    else
    {
        return '?';
    }
}

void get_time(char *str, time_t time)
{
    struct tm *time_gm = gmtime(&time);

    if (time_gm == NULL)
    {
        print_err(-1, "Error decoding time");
    }

    sprintf(str, "%02d/%02d/%02d %02d:%02d:%02d", time_gm->tm_mon + 1,
            time_gm->tm_mday,
            (time_gm->tm_year) % 100,
            time_gm->tm_hour,
            time_gm->tm_min,
            time_gm->tm_sec);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "image needed");
        exit(1);
    }
    char *image = argv[1];
    int fd = open(image, O_RDONLY);
    print_err(fd, "Could not open image");

    //super block summary
    struct ext2_super_block super;
    int read = pread(fd, &super, sizeof(super), 1024);
    if (read < 1024 && read != 0)
    {
        print_err(-1, "pread of superblock failed");
    }
    block_size = 1024 << super.s_log_block_size;
    fprintf(stdout, "SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", super.s_blocks_count, super.s_inodes_count, block_size, super.s_inode_size, super.s_blocks_per_group, super.s_inodes_per_group, super.s_first_ino);

    //group summary
    int num_groups = super.s_blocks_count / super.s_blocks_per_group + 1;
    struct ext2_group_desc group;

    for (int i = 0; i < num_groups; i++) //print for each group in file system
    {
        int group_offset = 1024 + block_size + i * sizeof(group);
        read = pread(fd, &group, sizeof(group), group_offset);
        print_err(read, "pread of group descriptors failed");

        int num_blocks_in_group = super.s_blocks_per_group;
        if (num_blocks_in_group > (int)super.s_blocks_count)
        {
            num_blocks_in_group = super.s_blocks_count;
        }
        fprintf(stdout, "GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", i, num_blocks_in_group, super.s_inodes_per_group, group.bg_free_blocks_count, group.bg_free_inodes_count, group.bg_block_bitmap, group.bg_inode_bitmap, group.bg_inode_table);

        //free block entries
        int block_bitmap = group.bg_block_bitmap;
        char *bitmap = (char *)malloc(block_size);
        int offset = calc_block_offset(block_bitmap);

        read = pread(fd, bitmap, block_size, offset);
        print_err(read, "pread of free block entries failed");

        for (unsigned int k = 0; k < block_size; k++)
        {
            char byte = bitmap[k];
            for (int j = 0; j < 8; j++)
            {
                int used_bit = byte & (1 << j);
                int free_block_num = (i * super.s_blocks_per_group) + (k * 8) + j + 1;
                if (used_bit == 0)
                {
                    fprintf(stdout, "BFREE,%d\n", free_block_num);
                }
            }
        }

        //free I-node entries
        int inode_bitmap = group.bg_inode_bitmap;
        char *inode_map = (char *)malloc(block_size);
        int i_offset = calc_block_offset(inode_bitmap);

        read = pread(fd, inode_map, block_size, i_offset);
        print_err(read, "pread of free i-node entries failed");
        for (unsigned int j = 0; j < block_size; j++)
        {
            char byte = inode_map[j];
            for (int k = 0; k < 8; k++)
            {
                int used_bit = byte & (1 << k);
                int free_block_num = (i * super.s_inodes_per_group) + (j * 8) + k + 1;
                if (used_bit == 0)
                {
                    fprintf(stdout, "IFREE,%d\n", free_block_num);
                }
            }
        }
        //I-node summary
        int inode_table_int = group.bg_inode_table;
        int table_size = super.s_inodes_per_group * sizeof(struct ext2_inode);
        struct ext2_inode *inode_table = (struct ext2_inode *)malloc(table_size);
        read = pread(fd, inode_table, table_size, calc_block_offset(inode_table_int));
        print_err(read, "failed to read inode table");
        for (unsigned int j = 0; j < super.s_inodes_per_group; j++)
        {
            struct ext2_inode curr = inode_table[j];
            if (curr.i_mode != 0 && curr.i_links_count != 0)
            {
                int i_num = super.s_inodes_per_group * i + j + 1;
                char f_type = get_filetype(curr.i_mode);
                __u16 mode = curr.i_mode & 0xFFF;
                char str_change[30];
                char str_mod[30];
                char str_access[30];
                get_time(str_change, curr.i_ctime);
                get_time(str_mod, curr.i_mtime);
                get_time(str_access, curr.i_atime);
                printf("INODE,%u,%c,%o,%u,%u,%u,%s,%s,%s,%u,%u", i_num, f_type, mode, curr.i_uid, curr.i_gid, curr.i_links_count, str_change, str_mod, str_access, curr.i_size, curr.i_blocks);
                if (f_type != 's')
                {
                    for (unsigned int k = 0; k < 15; k++)
                    {
                        fprintf(stdout, ",%d", curr.i_block[k]);
                    }
                }
                fprintf(stdout, "\n");
                if (f_type == 'd')
                {
                    //directory entries
                    struct ext2_dir_entry dir_entry;
                    unsigned int dir_offset = 0;
                    for (int m = 0; m < EXT2_NDIR_BLOCKS; m++)
                    {
                        if (curr.i_block[m] != 0)
                        {
                            while (dir_offset < block_size)
                            {
                                int loc = curr.i_block[m] * block_size + dir_offset;
                                read = pread(fd, &dir_entry, sizeof(dir_entry), loc);
                                print_err(read, "failed to read directory entries");

                                if (dir_entry.inode != 0)
                                {
                                    fprintf(stdout, "DIRENT,%u,%d,%u,%u,%u,'%s'\n", i_num, dir_offset, dir_entry.inode, dir_entry.rec_len, dir_entry.name_len, dir_entry.name);
                                }
                                dir_offset += dir_entry.rec_len;
                            }
                        }
                    }
                }
                //indirect block references
                if (curr.i_block[12] != 0)
                {
                    int *block_ptrs = malloc(block_size);
                    int block_offset = 1024 + (curr.i_block[12] - 1) * block_size;
                    read = pread(fd, block_ptrs, block_size, block_offset);
                    print_err(read, "could not read 12th block");

                    unsigned int num_block_ptrs = block_size / sizeof(int);
                    for (unsigned int j = 0; j < num_block_ptrs; j++)
                    {
                        if (block_ptrs[j] != 0)
                        {
                            if (f_type != 'd')
                            {
                                fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", i_num, 1, 12 + j, curr.i_block[12], block_ptrs[j]);
                            }
                            else
                            {
                                //read directory entries?
                            }
                        }
                    }
                    free(block_ptrs);
                }

                if (curr.i_block[13] != 0)
                {
                    int *ind_block_ptrs = malloc(block_size);
                    int block_offset = 1024 + (curr.i_block[13] - 1) * block_size;
                    read = pread(fd, ind_block_ptrs, block_size, block_offset);
                    print_err(read, "could not read 13th block");

                    for (unsigned int k = 0; k < block_size / sizeof(int); k++)
                    {
                        if (ind_block_ptrs[k] != 0)
                        {
                            fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", i_num, 2, 256 + 12 + k, curr.i_block[13], ind_block_ptrs[k]);

                            int *block_ptrs = malloc(block_size);
                            int block_offset_2 = 1024 + (ind_block_ptrs[k] - 1) * block_size;
                            read = pread(fd, block_ptrs, block_size, block_offset_2);
                            print_err(read, "could not read 13th block");

                            for (unsigned int m = 0; m < block_size / sizeof(int); m++)
                            {
                                if (block_ptrs[m] != 0)
                                {
                                    fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", i_num, 1, 256 + 12 + m, ind_block_ptrs[k], block_ptrs[m]);
                                }
                            }
                            free(block_ptrs);
                        }
                    }
                    free(ind_block_ptrs);
                }
                if (curr.i_block[14] != 0)
                {

                    int *ind_block_ptrs_2 = malloc(block_size);
                    int block_offset = 1024 + (curr.i_block[14] - 1) * block_size;
                    read = pread(fd, ind_block_ptrs_2, block_size, block_offset);
                    print_err(read, "could not read 14th block");

                    for (unsigned int k = 0; k < block_size / sizeof(int); k++)
                    {
                        if (ind_block_ptrs_2[k] != 0)
                        {
                            fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", i_num, 3, 65536 + 256 + 12 + k, curr.i_block[14], ind_block_ptrs_2[k]);

                            int *ind_block_ptrs = malloc(block_size);
                            int block_offset_2 = 1024 + (ind_block_ptrs_2[k] - 1) * block_size;
                            read = pread(fd, ind_block_ptrs, block_size, block_offset_2);
                            print_err(read, "could not read 14th block");

                            for (unsigned int m = 0; m < block_size / sizeof(int); m++)
                            {
                                if (ind_block_ptrs[m] != 0)
                                {
                                    fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", i_num, 2, 65536 + 256 + 12 + m, ind_block_ptrs_2[k], ind_block_ptrs[m]);
                                }

                                int *block_ptrs = malloc(block_size);
                                int block_offset_3 = 1024 + (ind_block_ptrs[m] - 1) * block_size;
                                read = pread(fd, block_ptrs, block_size, block_offset_3);
                                print_err(read, "could not read 13th block");

                                for (unsigned int n = 0; n < block_size / sizeof(int); n++)
                                {
                                    if (block_ptrs[n] != 0)
                                    {
                                        fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", i_num, 1, 65536 + 256 + 12 + n, ind_block_ptrs[m], block_ptrs[n]);
                                    }
                                }
                                free(block_ptrs);
                            }
                            free(ind_block_ptrs);
                        }
                    }
                    free(ind_block_ptrs_2);
                }
            }
        }

        free(inode_map);
        free(bitmap);
    }
}
