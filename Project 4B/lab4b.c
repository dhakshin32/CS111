/*
NAME: Dhakshin Suriakannu
EMAIL: bruindhakshin@g.ucla.edu
ID: 605280083
*/

#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <ctype.h>
#include <mraa.h>
#include <mraa/aio.h>

char scale = 'F';
int period = 1;
FILE *fptr;
int init_num = 1;
int GPIO = 60;

mraa_aio_context temperature;
mraa_gpio_context button;

struct timeval sys_clock;
time_t time_next = 0;
char *input;

int output = 1;

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
            printf("%02d:%02d:%02d ", tm->tm_hour, tm->tm_min, tm->tm_sec);

            printf("%d.%1d\n", t / 10, t % 10);
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

void do_when_pushed()
{
    struct tm *now = localtime(&sys_clock.tv_sec);

    printf("%02d:%02d:%02d SHUTDOWN\n", now->tm_hour, now->tm_min, now->tm_sec);

    if (fptr != NULL)
    {
        fprintf(fptr, "%02d:%02d:%02d SHUTDOWN\n", now->tm_hour, now->tm_min, now->tm_sec);
    }

    mraa_aio_close(temperature);
    mraa_gpio_close(button);
    exit(0);
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

    button = mraa_gpio_init(GPIO);

    if (button == NULL)
    {
        fprintf(stderr, "Failed to initialize GPIO_50\n");
        mraa_deinit();
        exit(1);
    }

    mraa_gpio_dir(button, MRAA_GPIO_IN);
    mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &do_when_pushed, NULL);
}

void print_command_str(char *str)
{
    if (fptr == NULL)
    {
        printf("%s\n", str);
    }
    else
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
    else if (strstr(str, "LOG") != NULL)
    {
        print_command_str(str);
    }
    else if (strcmp(str, "OFF") == 0)
    {
        print_command_str(str);
        do_when_pushed();
    }
    else
    {
        print_command_str(str);
    }
}
void commands(char *str)
{
    str[strlen(str) - 1] = '\0';
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
        char command[25];
        memcpy(command, &org[start], end - start);
        command[end - start] = '\0';
        process_command(command);
    }
}
int main(int argc, char *argv[])
{
    struct option options[] = {
        {"period", required_argument, NULL, 'p'},
        {"scale", required_argument, NULL, 's'},
        {"log", required_argument, NULL, 'l'},
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
        default:
            print_err(-1, "Invalid args\n");
            break;
        }
    }

    initialize_the_sensors();
    struct pollfd pollSTDIN;
    pollSTDIN.fd = 0;
    pollSTDIN.events = POLLIN;

    input = (char *)malloc(1024 * sizeof(char));
    if (input == NULL)
    {
        print_err(-1, "Could not malloc input buffer");
    }
    while (1)
    {
        read_temp();
        if (poll(&pollSTDIN, 1, 50))
        {
            fgets(input, 1024, stdin);
            commands(input);
        }
    }
    mraa_aio_close(temperature);
    mraa_gpio_close(button);
    exit(0);
}
