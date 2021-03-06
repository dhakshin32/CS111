/*
NAME: Dhakshin Suriakannu
EMAIL: bruindhakshin@g.ucla.edu
ID: 605280083
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <getopt.h>
#include <string.h>
#include <mraa.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ctype.h>
#include <mraa/aio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <unistd.h>

char scale = 'F';
int period = 1;
FILE *fptr;
int init_num = 1;
int GPIO = 60;

mraa_aio_context temperature;

struct timeval sys_clock;
time_t time_next = 0;
char input[1024];

int output = 1;

//new params
int socketfd;
char *id = "";
char *host = "";
int port = -1;

void exit_shutdown()
{
    struct tm *now = localtime(&sys_clock.tv_sec);

    dprintf(socketfd, "%02d:%02d:%02d SHUTDOWN\n", now->tm_hour, now->tm_min, now->tm_sec);
    if (fptr != NULL)
    {
        fprintf(fptr, "%02d:%02d:%02d SHUTDOWN\n", now->tm_hour, now->tm_min, now->tm_sec);
    }

    mraa_aio_close(temperature);
    exit(0);
}
void print_err(int x, char *message)
{
    if (x < 0)
    {
        fprintf(stderr, "%s", message);
        exit(1);
    }
}
float convert_temp(int reading)
{
    float R = 1023.0 / ((float)reading) - 1.0;
    R = 100000.0 * R;
    float C = 1.0 / (log(R / 100000.0) / 4275 + 1 / 298.15) - 273.15;
    float F = (C * 9) / 5 + 32;
    if (scale == 'F')
    {
        return F;
    }
    else
    {
        return C;
    }
}

void read_temp()
{
    gettimeofday(&sys_clock, 0);
    if (sys_clock.tv_sec >= time_next)
    {

        struct timespec ts;
        struct tm *tm;
        clock_gettime(CLOCK_REALTIME, &ts);
        tm = localtime(&(ts.tv_sec));
        int t = convert_temp(mraa_aio_read(temperature)) * 10;
        if (output)
        {
            dprintf(socketfd, "%02d:%02d:%02d %d.%1d\n", tm->tm_hour, tm->tm_min, tm->tm_sec, t / 10, t % 10);
        }

        time_next = sys_clock.tv_sec + period;

        if (fptr != NULL)
        {

            if (output)
            {
                fprintf(fptr, "%02d:%02d:%02d ", tm->tm_hour, tm->tm_min, tm->tm_sec);
                fprintf(fptr, "%d.%1d\n", t / 10, t % 10);
            }
        }
    }
}

void initialize_the_sensors()
{
    temperature = mraa_aio_init(init_num);

    if (temperature == NULL)
    {
        fprintf(stderr, "Failed to initialize AIO\n");
        mraa_deinit();
        exit(1);
    }
}

void print_command_str(char *str)
{
    if (fptr != NULL)
    {
        fprintf(fptr, "%s\n", str);
    }
}
void process_command(char *str)
{
    if (strcmp(str, "SCALE=F") == 0)
    {
        scale = 'F';
        print_command_str(str);
    }
    else if (strcmp(str, "SCALE=C") == 0)
    {
        scale = 'C';
        print_command_str(str);
    }
    else if (strstr(str, "PERIOD=") != NULL)
    {
        period = atoi(str + 7);
        print_command_str(str);
    }
    else if (strcmp(str, "STOP") == 0)
    {
        output = 0;
        print_command_str(str);
    }
    else if (strcmp(str, "START") == 0)
    {
        output = 1;
        print_command_str(str);
    }
    else if (strncmp(str, "LOG", strlen("LOG")) == 0)
    {
        print_command_str(str);
    }
    else if (strcmp(str, "OFF") == 0)
    {
        print_command_str(str);
        exit_shutdown();
    }
    else
    {
        print_command_str(str);
    }
}
void commands(char *str)
{
    char *org = str;
    char *end_char = str;
    char *start_char = str;
    int start = 0;
    int end = 0;

    while (*end_char != '\0')
    {
        while (*end_char != '\n' && *end_char != '\0')
        {

            end_char++;
            end++;
        }
        while (start < end && ((*start_char == ' ') || (*start_char == '\t')))
        {
            start_char++;
            start++;
        }
        char command[1024];
        memcpy(command, &org[start], end - start);
        command[end - start] = '\0';
        process_command(command);
        end_char++;
    }
}
int connect_client(char *host, int p)
{
    struct sockaddr_in serv_addr; //encode the ip address and the port for the remote server
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // AF_INET: IPv4, SOCK_STREAM: TCP connection
    struct hostent *server = gethostbyname(host);
    // convert host_name to IP addr
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET; //address is Ipv4
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    //copy ip address from server to serv_addr
    serv_addr.sin_port = htons(p);                                     //setup the port
    connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)); //initiate the connection to server
    return sockfd;
}
int main(int argc, char *argv[])
{
    struct option options[] = {
        {"period", required_argument, NULL, 'p'},
        {"scale", required_argument, NULL, 's'},
        {"log", required_argument, NULL, 'l'},
        {"host", required_argument, NULL, 'h'},
        {"id", required_argument, NULL, 'i'},
        {0, 0, 0, 0}};

    int opt;
    while ((opt = getopt_long(argc, argv, "", options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'p':
            period = atoi(optarg);
            break;
        case 's':
            if (optarg[0] == 'F' || optarg[0] == 'C')
            {
                scale = optarg[0];
                break;
            }
            break;
        case 'l':
            fptr = fopen(optarg, "a");
            if (fptr == NULL)
            {
                print_err(-1, "File for logging could not be created!");
            }
            break;
        case 'i':
            id = optarg;
            break;
        case 'h':
            host = optarg;
            break;
        default:
            print_err(-1, "Invalid args\n");
            break;
        }
    }
    if (fptr == NULL)
    {
        fprintf(stderr, "Failed to enter log file\n");
        exit(1);
    }
    if (strlen(id) != 9)
    {
        fprintf(stderr, "Failed to enter valid 9 digit ID\n");
        exit(1);
    }
    if (strlen(host) == 0)
    {
        fprintf(stderr, "Failed to enter host\n");
        exit(1);
    }
    if (optind < argc)
    {
        port = atoi(argv[optind]);
        if (port <= 0)
        {
            fprintf(stderr, "Invalid port\n");
            exit(1);
        }
    }

    socketfd = connect_client(host, port);

    if (fptr != NULL)
    {
        fprintf(fptr, "ID=%s\n", id);
    }
    dprintf(socketfd, "ID=%s\n", id);

    initialize_the_sensors();
    struct pollfd pollServer;
    pollServer.fd = socketfd;
    pollServer.events = POLLIN;

    while (1)
    {
        read_temp();
        if (poll(&pollServer, 1, 50))
        {
            int err = read(socketfd, input, 1024);
            if (err < 0)
            {
                fprintf(stderr, "Failed to read socket");
                exit(1);
            }
            input[err] = '\0';
            commands(input);
        }
    }
    mraa_aio_close(temperature);
    exit(0);
}
