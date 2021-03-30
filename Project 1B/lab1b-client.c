/*NAME : Dhakshin Suriakannu
EMAIL : bruindhakshin @g.ucla.edu
ID : 605280083*/
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <zlib.h>

z_stream comp_stream;
z_stream decomp_stream;
struct termios termios_p;
struct termios default_term;
int socket_fd;

void print_err(int err, char *message)
{
    if (err < 0)
    {
        fprintf(stderr, "Error: %s\r\n", message);
        exit(1);
    }
}
void reset_terminal()
{
    int set_err = tcsetattr(0, TCSANOW, &default_term);
    if (set_err < 0)
    {
        fprintf(stderr, "Error: Cannot reset terminal parameters\r\n");
        exit(1);
    }
}
void restore()
{
    close(socket_fd);
    reset_terminal();
}
void change_terminal(struct termios *termios_p)
{
    termios_p->c_iflag = ISTRIP;
    termios_p->c_oflag = 0;
    termios_p->c_lflag = 0;
    int set_err = tcsetattr(0, TCSANOW, termios_p);
    print_err(set_err, "cannot set terminal");
    set_err = atexit(restore);
    print_err(set_err, "cannot set atexit()");
}

void process_cmd_args(int argc, char *argv[], int *port, int *log, int *compress)
{
    static struct option long_options[] = {
        {"port", required_argument, NULL, 'p'},
        {"log", required_argument, NULL, 'l'},
        {"compress", no_argument, NULL, 'c'},
        {0, 0, 0, 0}};
    char c;
    while (1)
    {
        c = getopt_long(argc, argv, "p", long_options, NULL);
        if (c == -1)
        {
            break;
        }
        switch (c)
        {
        case 'p':
            *port = atoi(optarg);
            break;
        case 'l':
            if ((*log = creat(optarg, 0666)) == -1)
            {
                fprintf(stderr, "Error: Failed to create file.\n");
            }
            break;
        case 'c':
            *compress = 1;
            break;
        default:
            fprintf(stderr, "Error: Only --port, --log, and --compress are allowed.\n");
            exit(1);
        }
    }
    if (*port == -1)
    {
        fprintf(stderr, "Error: --port required\r\n");
        exit(1);
    }
}
void init_decompress_stream(z_stream *strm)
{
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
    strm->opaque = Z_NULL;
    if (inflateInit(strm) != Z_OK)
    {
        print_err(-1, "inflateInit() failed");
        exit(1);
    }
}
void init_compress_stream(z_stream *strm)
{
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
    strm->opaque = Z_NULL;
    if (deflateInit(strm, Z_DEFAULT_COMPRESSION) != Z_OK)
    {
        print_err(-1, "deflateInit() failed");
    }
}
void client_connect(unsigned int port)
{
    int err;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    print_err(socket_fd, "cannot make socket");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    server = gethostbyname("localhost");

    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length); /* copy ip address from server to serv_addr */
    memset(serv_addr.sin_zero, '\0', sizeof serv_addr.sin_zero);          /* padding zeros*/

    err = connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    print_err(err, "cannot connect to server");
}

int main(int argc, char *argv[])
{

    int port = -1;
    int log = -1;
    int compress = 0;

    process_cmd_args(argc, argv, &port, &log, &compress);

    client_connect(port);

    //set terminal
    int err = tcgetattr(0, &termios_p);
    print_err(err, "Cannot get terminal parameters");
    err = tcgetattr(0, &default_term);
    print_err(err, "Cannot get terminal parameters");
    change_terminal(&termios_p);

    if (compress)
    {
        init_compress_stream(&comp_stream);
        init_decompress_stream(&decomp_stream);
    }

    struct pollfd fddesc[] = {
        {0, POLLIN, 0},
        {socket_fd, POLLIN, 0}};

    while (1)
    {
        if (poll(fddesc, 2, 0) > 0)
        {
            int poll_stdin = fddesc[0].revents;
            int poll_socket = fddesc[1].revents;

            if (poll_socket == POLLIN)
            {

                //read from socket_fd, process special characters, send to stdout
                char buff[256];
                int read_count = read(socket_fd, &buff, 256);
                print_err(read_count, "Read failed");
                buff[read_count] = '\0';
                if (read_count == 0)
                {
                    exit(0);
                }
                if (compress == 1)
                {
                    //decompress
                    char decompression_buf[1024];
                    decomp_stream.avail_in = read_count;
                    decomp_stream.next_in = (unsigned char *)buff;
                    decomp_stream.avail_out = 1024;
                    decomp_stream.next_out = (unsigned char *)decompression_buf;

                    do
                    {
                        inflate(&decomp_stream, Z_SYNC_FLUSH);
                    } while (decomp_stream.avail_in > 0);

                    for (int i = 0; (unsigned int)i < 1024 - decomp_stream.avail_out; i++)
                    {
                        char c;
                        c = decompression_buf[i];
                        if (c == '\r' || c == '\n')
                        {
                            char carrnewline[3] = {'\r', '\n', '\0'};
                            write(1, carrnewline, 3);
                        }
                        else
                        {
                            write(1, &c, 1);
                        }
                    }
                }
                else
                {
                    int err = write(1, &buff, read_count);
                    print_err(err, "Write failed");
                }
                if (log)
                {
                    char num_bytes[20];
                    sprintf(num_bytes, "%d", read_count);

                    write(log, "RECEIVED ", 9);
                    write(log, num_bytes, strlen(num_bytes));
                    write(log, " bytes: ", 8);
                    write(log, buff, read_count);
                    write(log, "\n", 1);
                }
            }
            else if (poll_stdin == POLLIN)
            {
                //read from stdin, process special characters, send to stdout and socket_fd
                char buff[256];
                int read_count = read(0, &buff, 256);
                print_err(read_count, "Read failed");
                buff[read_count] = '\0';
                char compression_buf[256];
                if (compress)
                {
                    //compress

                    comp_stream.avail_in = read_count;
                    comp_stream.next_in = (unsigned char *)buff;
                    comp_stream.avail_out = 256;
                    comp_stream.next_out = (unsigned char *)compression_buf;
                    do
                    {
                        deflate(&comp_stream, Z_SYNC_FLUSH);
                    } while (comp_stream.avail_in > 0);
                    write(socket_fd, compression_buf, 256 - comp_stream.avail_out);
                }
                for (int i = 0; i < read_count; i++)
                {
                    char c;
                    c = buff[i];
                    if (c == '\r' || c == '\n')
                    {
                        char carrnewline[3] = {'\r', '\n', '\0'};
                        char shellnewline[2] = {'\n', '\0'};
                        int err = write(1, &carrnewline, 3);
                        print_err(err, "Write failed");
                        if (compress == 0)
                        {
                            err = write(socket_fd, &shellnewline, 2);
                            print_err(err, "Write failed");
                        }
                    }
                    else
                    {
                        int err = write(1, &c, 1);
                        print_err(err, "Write failed");
                        if (compress == 0)
                        {
                            err = write(socket_fd, &c, 1);
                            print_err(err, "Write failed");
                        }
                    }
                }
                if (log && compress)
                {
                    char num_bytes[20];
                    sprintf(num_bytes, "%d", 256 - comp_stream.avail_out);

                    write(log, "SENT ", 5);
                    write(log, num_bytes, strlen(num_bytes));
                    write(log, " bytes: ", 8);
                    write(log, compression_buf, 256 - comp_stream.avail_out);
                    write(log, "\n", 1);
                }
                else if (log)
                {
                    char num_bytes[20];
                    sprintf(num_bytes, "%d", read_count);

                    write(log, "SENT ", 5);
                    write(log, num_bytes, strlen(num_bytes));
                    write(log, " bytes: ", 8);
                    write(log, buff, read_count);
                    write(log, "\n", 1);
                }
            }
            else if (poll_stdin & POLLERR)
            {
                fprintf(stderr, "Error polling (from socket): %s\n", strerror(errno));
                exit(1);
            }
            else if (poll_socket & POLLERR || poll_socket & POLLHUP)
            {
                if (compress)
                {
                    inflateEnd(&decomp_stream);
                    deflateEnd(&comp_stream);
                }
                if (log)
                {
                    close(log);
                }
                exit(0);
            }
        }
    }

    if (compress)
    {
        inflateEnd(&decomp_stream);
        deflateEnd(&comp_stream);
    }
    if (log)
    {
        close(log);
    }

    exit(0);
}