/*
NAME: Dhakshin Suriakannu
EMAIL: bruindhakshin@g.ucla.edu
ID: 605280083
*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>

int term_shell[2];
int shell_term[2];
pid_t pid;

void shell_exit()
{
    int status;
    waitpid(pid, &status, 0);
fprintf(stderr,"SHELL EXIT SIGNAL=%d STATUS=%d", WTERMSIG(status), WEXITSTATUS(status));
}
void close_seqeunce()
{
    close(term_shell[1]);
    close(shell_term[0]);
    kill(pid, SIGINT);
    shell_exit();
    exit(0);
}
void create_pipes(int *term_shell, int *shell_term)
{
    if (pipe(term_shell) == -1)
    {
        fprintf(stderr, "Error: Pipe Failed\r\n");
        exit(1);
    }
    if (pipe(shell_term) == -1)
    {
        fprintf(stderr, "Error: Pipe Failed\r\n");
        exit(1);
    }
}
void change_terminal(struct termios *termios_p)
{
    termios_p->c_iflag = ISTRIP;
    termios_p->c_oflag = 0;
    termios_p->c_lflag = 0;
    int set_err = tcsetattr(0, TCSANOW, termios_p);
    if (set_err < 0)
    {
        fprintf(stderr, "Error: Cannot set terminal parameters\r\n");
        exit(1);
    }
}
void reset_terminal(struct termios *default_term)
{

    int set_err = tcsetattr(0, TCSANOW, default_term);
    if (set_err < 0)
    {
        fprintf(stderr, "Error: Cannot reset terminal parameters\r\n");
        exit(1);
    }
    exit(0);
}
int main(int argc, char *argv[])
{
    char c;
    int shell_bool=0;
    static struct option long_options[] = {
          {"shell", no_argument, NULL, 's'},
          {0,0,0,0}
            };
    while (1)
    {
        c = getopt_long(argc, argv, "", long_options, NULL);
	if(c==-1){
	  break;
	}
switch (c) {
 case 's': 
				shell_bool = 1;
				break;
			default:
				fprintf(stderr, "Error: Only --shell is allowed.\n");
				exit(1);
		}       
       
    }

        struct termios termios_p;
        struct termios default_term;
        int get_err = tcgetattr(0, &termios_p);
        if (get_err < 0)
        {
            fprintf(stderr, "Error: Cannot get terminal parameters\r\n");
            exit(1);
        }

        get_err = tcgetattr(0, &default_term);
        if (get_err < 0)
        {
            fprintf(stderr, "Error: Cannot get terminal parameters\r\n");
            exit(1);
        }

        //change terminal
        change_terminal(&termios_p);

        if (shell_bool)
        {

            signal(SIGPIPE, close_seqeunce);
            create_pipes(term_shell, shell_term);
            pid = fork();
            if (pid == -1)
            {
                fprintf(stderr, "Error: Can't fork\r\n");
                exit(1);
            }
            else if (pid == 0)
            {
                //child
                close(term_shell[1]);
                close(shell_term[0]);

                close(0);
                dup(term_shell[0]);
                close(term_shell[0]);

                close(1);
                dup(shell_term[1]);
                close(shell_term[1]);

                dup2(1, 2);
                int err = execlp("/bin/bash", "bash", NULL);
                if (err < -1)
                {
                    fprintf(stderr, "Error: Failed to exec a shell.\n");
                    exit(1);
                }
            }
            else
            {
                //parent
                close(term_shell[0]);
                close(shell_term[1]);
                struct pollfd fddesc[] = {
                    {0, POLLIN, 0},
                    {shell_term[0], POLLIN, 0}};

                while (1)
                {
                    if (poll(fddesc, 2, 0) > 0)
                    {
                        int keyboard = fddesc[0].revents;
                        int shell = fddesc[1].revents;
                        if (keyboard == POLLIN)
                        {
                            char buff[256];
                            int read_count = read(0, &buff, 256);
                            if (read_count < 0)
                            {
                                fprintf(stderr, "Error: Read failed\r\n");
                                exit(1);
                            }
                            buff[read_count] = '\0';

                            for (int i = 0; i < read_count; i++)
                            {
                                char c = buff[i];
                                if (c == 4)
                                {
                                    
                                    close(term_shell[1]);
                                    shell_exit();
                                    reset_terminal(&default_term);
                                }
                                else if (c == 3)
                                {
                                   int err= kill(pid, SIGINT);
                                   if (err < 0)
                                   {
                                       fprintf(stderr, "Error: Kill failed\r\n");
                                       exit(1);
                                   }
                                }
                                else if (c == '\r' || c == '\n')
                                {
                                    char carrnewline[3] = {'\r', '\n', '\0'};
                                    char shellnewline[2] = {'\n', '\0'};
                                    int err = write(1, &carrnewline, 3);
                                    if (err < 0)
                                    {
                                        fprintf(stderr, "Error: Write failed\r\n");
                                        exit(1);
                                    }
                                    err = write(term_shell[1], &shellnewline, 2);
                                    if (err < 0)
                                    {
                                        fprintf(stderr, "Error: Write failed\r\n");
                                        exit(1);
                                    }
                                }
                                else
                                {
                                    int err = write(1, &c, 1);
                                    if (err < 0)
                                    {
                                        fprintf(stderr, "Error: Write failed\r\n");
                                        exit(1);
                                    }
                                    err = write(term_shell[1], &c, 1);
                                    if (err < 0)
                                    {
                                        fprintf(stderr, "Error: Write failed\r\n");
                                        exit(1);
                                    }
                                }
                            }
                        }
                        else if (keyboard == POLLERR || keyboard == POLLHUP)
                        {
                            //error
                            fprintf(stderr, "Error: Poll failed for keyboard input.\n");
                            exit(1);
                        }
                        else if (shell == POLLIN)
                        {
                            char buff[256];
                            int read_count = read(shell_term[0], &buff, 256);
                            if (read_count < 0)
                            {
                                fprintf(stderr, "Error: Read failed\r\n");
                                exit(1);
                            }
                            buff[read_count] = '\0';

                            for (int i = 0; i < read_count; i++)
                            {
                                char c = buff[i];
                                if (c == 4)
                                {
                                    reset_terminal(&default_term);
                                }
                                else if (c == '\n')
                                {

                                    char carrnewline[3] = {'\r', '\n', '\0'};
                                    int err = write(1, &carrnewline, 3);
                                    if (err < 0)
                                    {
                                        fprintf(stderr, "Error: Write failed\r\n");
                                        exit(1);
                                    }
                                }
                                else
                                {
                                    int err = write(1, &c, 1);
                                    if (err < 0)
                                    {
                                        fprintf(stderr, "Error: Write failed\r\n");
                                        exit(1);
                                    }
                                }
                            }
                        }
                        else if (shell == POLLERR || shell == POLLHUP)
                        {

			  shell_exit();
                            close(shell_term[0]);
                           
			    reset_terminal(&default_term);
                            exit(0);
                        }
                    }
                }
            }
        }
        else
        {
            while (1)
            {
                char buff[256];
                int read_count = read(0, &buff, 256);
                if (read_count < 0)
                {
                    fprintf(stderr, "Error: Read failed\r\n");
                    exit(1);
                }
                buff[read_count] = '\0';

                for (int i = 0; i < read_count; i++)
                {
                    char c = buff[i];
                    if (c == 4)
                    {
                        reset_terminal(&default_term);
                    }
                    else if (c == '\r' || c == '\n')
                    {
                        char carrnewline[3] = {'\r', '\n', '\0'};
                        int err = write(1, &carrnewline, 3);
                        if (err < 0)
                        {
                            fprintf(stderr, "Error: Write failed\r\n");
                            exit(1);
                        }
                    }
                    else
                    {
                        int err = write(1, &c, 1);
                        if (err < 0)
                        {
                            fprintf(stderr, "Error: Write failed\r\n");
                            exit(1);
                        }
                    }
                }
            }
        }
    }
