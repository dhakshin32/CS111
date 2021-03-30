#NAME: Shrea Chari
#EMAIL: shreachari@gmail.com
#ID: 005318456
#NAME: Dhakshin Suriakannu
#EMAIL: dhakshin.s@g.ucla.edu
#ID: 605280083

import sys, math, numpy, csv

my_superblock = None
my_group = None
my_inode = []
my_dirent = []
my_indirect = []
my_bfree = []
my_ifree = []
my_map = {}
my_dupblocks = set()

num_blocks = 0;
first_block = 0;

def print_error(message):
    sys.stderr.write(message)
    sys.exit(1)

class superblock:
    def __init__(self, item):
        self.s_blocks_count = int(item[1])
        self.s_inodes_count = int(item[2])
        self.block_size = int(item[3])
        self.s_inode_size = int(item[4])
        self.s_blocks_per_group = int(item[5])
        self.s_inodes_per_group = int(item[6])
        self.s_first_ino = int(item[7])

class group:
    def __init__(self, item):
        self.s_inodes_per_group = int(item[3])
        self.bg_inode_table = int(item[-1])

class inode:
    def __init__(self, item):
        self.inode_num = int(item[1])
        self.file_type = item[2]
        self.mode = int(item[3])
        self.i_uid = int(item[4])
        self.i_gid = int(item[5])
        self.i_links_count = int(item[6])
        self.ctime_str = item[7]
        self.mtime_str = item[8]
        self.atime_str = item[9]
        self.i_size = int(item[10])
        self.i_blocks = int(item[11])
        self.dirent_blocks = [int(num) for num in item[12:24]]
        self.indirect_blocks = [int(num) for num in item[24:27]]

class dirent:
    def __init__(self,item):
        self.inode_num = int(item[1])
        self.dir_offset = int(item[2])
        self.inode = int(item[3])
        self.rec_length = int(item[4])
        self.name_length = int(item[5])
        self.name = item[6]

class indirect:
    def __init__(self,item):
        self.inode_num = int(item[1])
        self.level = int(item[2])
        self.offset = int(item[3])
        self.block_number = int(item[4])
        self.element = int(item[5])



def main ():
    #check number of arguments
    if len(sys.argv) != 2:
        print_error("Number of input arguments must be 2.\n")
    
    #open csv file
    try:
        csv_input = open(sys.argv[1], 'r')
    except IOError:
        print_error("Failure to open the csv file inputted.\n")
    

    #read csv input
    reader = csv.reader(csv_input)
    for i in reader:
        if i[0] == "SUPERBLOCK":
            my_superblock = superblock(i)
        elif i[0] == "GROUP":
            my_group = group(i)
        elif i[0] == "INODE":
            my_inode.append(inode(i))
        elif i[0] == "DIRENT":
            my_dirent.append(dirent(i))
        elif i[0] == "INDIRECT":
            my_indirect.append(indirect(i))
        elif i[0] == "BFREE":
            my_bfree.append(int(i[1]))
        elif i[0] == "IFREE":
            my_ifree.append(int(i[1]))
        else:
            print_error("Invalid element(s) found in csv file.\n")
    
    num_blocks = my_superblock.s_blocks_count
    first_block = int(math.ceil(my_group.s_inodes_per_group * my_superblock.s_inode_size / my_superblock.block_size) + my_group.bg_inode_table)

    #look at the blocks now and report any errors
    #look at inode table
    for i in my_inode:
        if i.i_size <= num_blocks and i.file_type == 's':
            continue
        index = 0;
        for num in i.dirent_blocks:
            if num >= num_blocks or num < 0:
                print("INVALID BLOCK %d IN INODE %d AT OFFSET %d" %(num, i.inode_num, index))
            elif num < first_block and num > 0:
                print("RESERVED BLOCK %d IN INODE %d AT OFFSET %d" %(num, i.inode_num, index))
            elif num != 0:
                if num not in my_map:
                    my_map[num] = [[i.inode_num, index, 0]]
                else:
                    my_dupblocks.add(num)
                    my_map[num].append([i.inode_num, index, 0])
            index += 1

        #sinlge indirect block
        num = i.indirect_blocks[0]
        if num >= num_blocks or num < 0:
            print("INVALID INDIRECT BLOCK %d IN INODE %d AT OFFSET %d" %(num, i.inode_num, 12))
        elif num < first_block and num > 0:
            print("RESERVED INDIRECT BLOCK %d IN INODE %d AT OFFSET %d" %(num, i.inode_num, 12))
        elif num != 0:
            if num not in my_map:
                my_map[num] = [[i.inode_num, 12, 1]]
            else:
                my_dupblocks.add(num)
                my_map[num].append([i.inode_num, 12, 1])

        #double indirect block
        num = i.indirect_blocks[1]
        if num >= num_blocks or num < 0:
            print("INVALID DOUBLE INDIRECT BLOCK %d IN INODE %d AT OFFSET %d" %(num, i.inode_num, 268))
        elif num < first_block and num > 0:
            print("RESERVED DOUBLE INDIRECT BLOCK %d IN INODE %d AT OFFSET %d" %(num, i.inode_num, 268))
        elif num != 0:
            if num not in my_map:
                my_map[num] = [[i.inode_num, 268, 2]]
            else:
                my_dupblocks.add(num)
                my_map[num].append([i.inode_num, 268, 2])

        #triple indirect block
        num = i.indirect_blocks[2]
        if num >= num_blocks or num < 0:
            print("INVALID TRIPLE INDIRECT BLOCK %d IN INODE %d AT OFFSET %d" %(num, i.inode_num, 65804))
        elif num < first_block and num > 0:
            print("RESERVED TRIPLE INDIRECT BLOCK %d IN INODE %d AT OFFSET %d" %(num, i.inode_num, 65804))
        elif num != 0:
            if num not in my_map:
                my_map[num] = [[i.inode_num, 65804, 3]]
            else:
                my_dupblocks.add(num)
                my_map[num].append([i.inode_num, 65804, 3])

    for item in my_indirect:
        num = item.element
        level_name = ""
        if item.level == 1:
            level_name = "INDIRECT"
        elif item.level == 2:
            level_name = "DOUBLE INDIRECT"
        elif item.level == 3:
            level_name = "TRIPLE INDIRECT"

        if num >= num_blocks or num < 0:
            print("INVALID %s BLOCK %d IN INODE %d AT OFFSET %d" %(level_name, num, item.inode_num, item.offset))
        elif num < first_block and num > 0:
            print("RESERVED %s INDIRECT BLOCK %d IN INODE %d AT OFFSET %d" %(level_num, num, item.inode_num, item.offset))
        elif num != 0:
            if num not in my_map:
                my_map[num] = [[item.inode_num, item.offset, item.level]]
            else:
                my_dupblocks.add(num)
                my_map[num].append([item.inode_num, item.offset, item.level])

    #look for unreferenced or allocated blocks
    for num in range(first_block, num_blocks):
        if num not in my_map and num not in my_bfree:
            print("UNREFERENCED BLOCK %d" %num)
        if num in my_map and num in my_bfree:
            print("ALLOCATED BLOCK %d ON FREELIST" %num)

    #print dup block info
    for num in my_dupblocks:
        for item in my_map[num]:
            level = ""
            if item[-1] == 1:
                level = "INDIRECT"
            elif item[-1] == 2:
                level = "DOUBLE INDIRECT"
            elif item[-1] == 3:
                level = "TRIPLE INDIRECT"
            if level == "":
                print("DUPLICATE BLOCK %d IN INODE %d AT OFFSET %d" %(num, item[0], item[1]))
            else:
                print("DUPLICATE %s BLOCK %d IN INODE %d AT OFFSET %d" %(level, num, item[0], item[1]))

    #look at the inode and directory now and report any errors
    inodes_allocated = []
    link = numpy.zeros(my_superblock.s_inodes_count + my_superblock.s_first_ino)
    parent = numpy.zeros(my_superblock.s_inodes_count + my_superblock.s_first_ino)
    parent[2]=2

    #print inode number when valid and free
    for i in my_inode:
        if i.inode_num != 0:
            inodes_allocated.append(i.inode_num)
            if i.inode_num in my_ifree:
                print("ALLOCATED INODE %d ON FREELIST" %(i.inode_num))

    #scan valid inodes to detect unallocated inodes not free
    for i in range(my_superblock.s_first_ino, my_superblock.s_inodes_count):
        if i not in inodes_allocated and i not in my_ifree:
            print("UNALLOCATED INODE %d NOT ON FREELIST" %(i))

    for i in my_inode:
        if i.file_type == '0' and i.inode_num not in my_ifree:
            print("UNALLOCATED INOFE %d NOT ON FREELIST" %(i.inode_num))

    for d in my_dirent:
        if d.inode < 1 or d.inode > my_superblock.s_inodes_count:
            print("DIRECTORY INODE %d NAME %s INVALID INODE %d" %(d.inode_num, d.name, d.inode))
        elif d.inode not in inodes_allocated:
            print("DIRECTORY INODE %d NAME %s UNALLOCATED INODE %d" %(d.inode_num, d.name, d.inode))
        else:
            link[d.inode] += 1

    for i in my_inode:
        if link[i.inode_num] != i.i_links_count:
            print("INODE %d HAS %d LINKS BUT LINKCOUNT IS %d" %(i.inode_num, link[i.inode_num], i.i_links_count))

    for d in my_dirent:
        if d.name != "'..'" and d.name != "'.'":
            parent[d.inode] = d.inode_num

    for d in my_dirent:
        if d.name == "'.'" and d.inode != d.inode_num:
            print("DIRECTORY INODE %d NAME '.' LINK TO INODE %d SHOULD BE %d" %(d.inode_num, d.inode, d.inode_num))
        if d.name == "'..'" and d.inode != parent[d.inode_num]:
            print("DIRECTORY INODE %d NAME '..' LINK TO INODE %d SHOULD BE %d" %(d.inode_num, d.inode, parent[d.inode_num]))

if __name__ == '__main__':
    main()