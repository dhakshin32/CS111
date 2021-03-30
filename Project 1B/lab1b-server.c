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
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <zlib.h>

int old_sockfd, socket_fd;
int from_shell[2];
int to_shell[2];
pid_t pid;
z_stream decomp_stream;
z_stream comp_stream;

void print_err(int err, char *message)
{
    if (err < 0)
    {
        fprintf(stderr, "Error: %s\r\n", message);
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
void shell_exit()
{
    int status;
    waitpid(pid, &status, 0);
    fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\r\n", WTERMSIG(status), WEXITSTATUS(status));
    close(socket_fd);
}
void sigpipe_handle()
{
    close(from_shell[0]);
    close(to_shell[1]);
    kill(pid, SIGKILL);
    shell_exit();
    exit(0);
}
void create_pipes(int *from_shell, int *to_shell)
{
    if (pipe(from_shell) == -1)
    {
        fprintf(stderr, "Error: Pipe Failed\r\n");
        exit(1);
    }
    if (pipe(to_shell) == -1)
    {
        fprintf(stderr, "Error: Pipe Failed\r\n");
        exit(1);
    }
}

void process_cmd_args(int argc, char *argv[], int *port, int *compress)
{
    static struct option long_options[] = {
        {"port", required_argument, NULL, 'p'},
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
int server_connect(int port)
{
    /* listen on sock_fd, new connection on new_fd */
    struct sockaddr_in my_addr;    /* my address */
    struct sockaddr_in their_addr; /* connector addr */
    socklen_t sin_size;
    /* create a socket */
    old_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    /* set the address info */
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port); /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY;
    /* INADDR_ANY allows clients to connect to any one of the host’s IP address.*/
    memset(my_addr.sin_zero, '\0', sizeof(my_addr.sin_zero)); //padding zeros
    /* bind the socket to the IP address and port number */
    bind(old_sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));

    listen(old_sockfd, 5); /* maximum 5 pending connections */
    sin_size = sizeof(struct sockaddr_in);
    /* wait for client’s connection, their_addr stores client’s address */
    socket_fd = accept(old_sockfd, (struct sockaddr *)&their_addr, &sin_size);
    return socket_fd; /* new_fd is returned not sock_fd*/
}
int main(int argc, char *argv[])
{
    //process args and connect server
    int port = -1;
    int compress = 0;
    process_cmd_args(argc, argv, &port, &compress);
    socket_fd = server_connect(port);
    if (compress)
    {

        init_compress_stream(&comp_stream);
        init_decompress_stream(&decomp_stream);
    }
    //sigpipe handler, create pipes, and fork
    signal(SIGPIPE, sigpipe_handle);
    create_pipes(from_shell, to_shell);
    pid = fork();

    if (pid == 0) //child
    {
        int err;
        err = close(to_shell[1]);
        print_err(err, "Close failed");
        err = close(from_shell[0]);
        print_err(err, "Close failed");

        //stdin
        err = close(0);
        print_err(err, "Close failed");
        err = dup(to_shell[0]);
        print_err(err, "dup failed");
        err = close(to_shell[0]);
        print_err(err, "Close failed");

        //stdout
        err = close(1);
        print_err(err, "Close failed");
        err = dup(from_shell[1]);
        print_err(err, "dup failed");
        err = close(from_shell[1]);
        print_err(err, "Close failed");

        //stderr
        err = dup2(1, 2);
        print_err(err, "dup failed");

        err = execlp("/bin/bash", "bash", NULL);
        print_err(err, "Failed to exec a shell");
    }
    else if (pid > 0) //parent
    {
        close(to_shell[0]);
        close(from_shell[1]);
        struct pollfd fddesc[] = {
            {socket_fd, POLLIN, 0},
            {from_shell[0], POLLIN, 0}};
        int p = 1;
        while (1)
        {
            if (poll(fddesc, 2, 0) > 0)
            {
                int poll_socket = fddesc[0].revents;
                int poll_from_shell = fddesc[1].revents;

                if (poll_socket == POLLIN)
                {
                    //read from socket_fd, process special characters, send to to_shell[1]
                    int err;
                    char buff[256];
                    int read_count = read(socket_fd, &buff, 256);
                    print_err(read_count, "Read failed");
                    buff[read_count] = '\0';
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

                        for (unsigned int i = 0; i < 1024 - decomp_stream.avail_out; i++)
                        {
                            char c;
                            c = decompression_buf[i];
                            if (c == 4)
                            {

                                err = close(to_shell[1]);
                                print_err(err, "Close to_shell[1] failed");
                                p = 0;
                                break;
                            }
                            else if (c == 3)
                            {
                                //p = 0;

                                err = kill(pid, SIGINT);
                                print_err(err, "Kill failed");
                            }
                            else if (c == '\r' || c == '\n')
                            {
                                char shellnewline[2] = {'\n', '\0'};
                                err = write(to_shell[1], &shellnewline, 2);
                                print_err(err, "Write failed");
                            }
                            else
                            {
                                err = write(to_shell[1], &c, 1);
                                print_err(err, "Write failed");
                            }
                        }
                    }
                    else if (p == 1)
                    {
                        for (int i = 0; i < read_count; i++)
                        {
                            char c = buff[i];
                            if (c == 4)
                            {

                                err = close(to_shell[1]);
                                print_err(err, "Close to_shell[1] failed");
                                p = 0;
                                break;
                            }
                            else if (c == 3)
                            {
                                err = kill(pid, SIGINT);
                                print_err(err, "Kill failed");
                            }
                            else if (c == '\r' || c == '\n')
                            {
                                char shellnewline[2] = {'\n', '\0'};
                                err = write(to_shell[1], &shellnewline, 2);
                                print_err(err, "Write failed");
                            }
                            else
                            {
                                err = write(to_shell[1], &c, 1);
                                print_err(err, "Write failed");
                            }
                        }
                    }
                }

                if (poll_from_shell == POLLIN)
                {
                    //read from from_shell[0], process special characters, send to socket_fd

                    int err;
                    char buff[256];
                    int read_count = read(from_shell[0], &buff, 256);
                    print_err(read_count, "Read failed");
                    buff[read_count] = '\0';
                    if (compress == 1)
                    {
                        //compress
                        char compression_buf[256];
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
                    else
                    {
                        for (int i = 0; i < read_count; i++)
                        {
                            char c = buff[i];
                            if (c == 4)
                            {
                                p = 0;
                                err = close(to_shell[1]);
                                print_err(err, "Close failed ");
                                break;
                            }
                            else if (c == 3)
                            {
                                err = kill(pid, SIGINT);
                                print_err(err, "Kill failed");
                                break;
                            }
                            else if (c == '\r' || c == '\n')
                            {
                                char carrnewline[3] = {'\r', '\n', '\0'};
                                err = write(socket_fd, &carrnewline, 3);
                                print_err(err, "Write failed");
                            }
                            else
                            {
                                err = write(socket_fd, &c, 1);
                                print_err(err, "Write failed");
                            }
                        }
                    }
                }
                if (poll_socket & POLLERR)
                {
                    fprintf(stderr, "Error polling (from socket): %s\n", strerror(errno));
                    exit(1);
                }
                if (poll_from_shell & POLLERR || poll_from_shell & POLLHUP)
                {
                    p = 0;
                }
            }
            if (p == 0)
            {
                break;
            }
        }
        close(socket_fd);
        close(old_sockfd);
        close(from_shell[0]);
        if (compress)
        {
            inflateEnd(&decomp_stream);
            deflateEnd(&comp_stream);
        }
        shell_exit();
    }
    exit(0);
}