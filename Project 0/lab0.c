//NAME: Dhakshin Suriakannu
//EMAIL: bruindhakshin@g.ucla.edu
//ID: 605280083
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>


void sighandler(int signum) {
  fprintf(stderr,"Segmentation fault caught: Caught signal %d\n", signum);
   exit(4);
}


int main(int argc, char *argv[])
{
    int c;
    char *input = "-1";
    char *output = "-1";
    int seg = 0;
    char *segfault=NULL;
    int catch = 0;
    while (1)
    {
        int option_index = 0;
        static struct option long_options[] = {
            {"input", required_argument, NULL, 0},
            {"output", required_argument, NULL, 0},
            {"segfault", no_argument, NULL, 0},
            {"catch", no_argument, NULL, 0}};

        c = getopt_long(argc, argv, "", long_options, &option_index);
        if (c == -1)
        {
            break;
        }
        else if (c == 0)
        {
            if (strcmp("input", long_options[option_index].name) == 0)
            {
                input = optarg;
            }
            else if (strcmp("output", long_options[option_index].name) == 0)
            {
                output = optarg;
            }
            else if (strcmp("segfault", long_options[option_index].name) == 0)
            {
	      seg = 1;
            }
            else if (strcmp("catch", long_options[option_index].name) == 0)
            {
                catch = 1;
            }
        }
        else if (c == '?')
        {
            fprintf(stderr, "Illegal option: Accepted options are --input, --output, --segfault, --catch\n");
            exit(1);
        }
    }
    
    if (seg == 1)
    {
    
      if(catch==1){
	signal(SIGSEGV,sighandler);
	printf("%c",segfault[0]);
	exit(4);
      }else{
	 printf("%c",segfault[0]);
      }
       
	
    }
    if (strcmp("-1", input) == 0)
    {
        //read from stdin
        char buff[128];
        int bytes;
        do
        {
	  bytes  = read(0, &buff, 128);
            if (bytes == 0)
            {
                //EOF
                break;
            }
            if(bytes<0){
                //error
	      fprintf(stderr,"Failed to read from stdin, error:%s\n",strerror(errno));
            }
	    
            if (strcmp("-1", output) == 0)
            {
                //output to stdout
                int err = write(1,buff,bytes);
		if(err<0){
		  //error
		  fprintf(stderr,"Failed to output to stdout, error:%s\n",strerror(errno));
		  
		}
            }
            else
            {
                //output to file
	      FILE *fout = fopen(output, "a");
            if (fout == NULL)
            {
	      fprintf(stderr,"Problem with argument --output: Failed to create file called %s, error:%s\n",output, strerror(errno));
	      //error
	      exit(3);
            }
            int fd = fileno(fout);
            int err = write(fd, buff, bytes);
	    if(err<0){
	      fprintf(stderr,"Problem with argument --output: Failed to write to file called %s, error:%s\n",output,strerror(errno));
	      exit(1);
	    }
            }

        } while (bytes > -1);
    }
    else
    {
        //read from file
        char *buff;
        FILE *fptr;
        long fsize;

        fptr = fopen(input, "r");
	if(fptr==NULL){
	  //error
	  fprintf(stderr,"Arugment --input failed because %s could not be opened. Error:%s\n",input, strerror(errno));
	  exit(2);
	 
	}
            if (fseek(fptr, 0, SEEK_END) == 0)
            {
                fsize = ftell(fptr);
                if (fsize == -1)
                {
                    //error	 
                    exit(1);
                }
                buff = malloc(sizeof(char) * (fsize + 1));
                if (fseek(fptr, 0, SEEK_SET) != 0)
                { /* Error */
                }
                size_t newLen = fread(buff, sizeof(char), fsize, fptr);
                if (ferror(fptr) != 0)
                {
                 
                  fprintf(stderr,"Arugment --input failed because error reading file %s. Error:%s\n",input, strerror(errno));
		  
                }
                else
                {
                    buff[newLen++] = '\0'; /* Just to be safe. */
                }
            }
            fclose(fptr);
        

        if (strcmp("-1", output) == 0)
        {
            //output to stdout
            int err = write(1, buff, fsize);
	    if(err<0){
fprintf(stderr,"Failed to output to stdout, error:%s\n",strerror(errno));
 exit(1);
	    }
	}
        else
        {
            //output to file
            FILE *fout = fopen(output, "w");
            if (fout == NULL)
            {
                //error
	       fprintf(stderr,"Problem with argument --output: Failed to create file called %s, error:%s\n",output, strerror(errno));
	       exit(3);
	    }
            int fd = fileno(fout);
            int outsize = write(fd, buff, fsize);
	    if(outsize<0){
fprintf(stderr,"Problem with argument --output: Failed to write to file called %s, error:%s\n",output,strerror(errno));
              exit(3);
	    }
        }
    }
    exit(0);
}
