#NAME: Dhakshin Suriakannu
#EMAIL: bruindhakshin@g.ucla.edu
#ID: 605280083

CC=gcc
flags=-g -lm -lmraa -Wall -Wextra
default:
	$(CC) $(flags) lab4b.c -o lab4b
check: default
	chmod u+x test.sh
	./test.sh	
	rm -f out.txt
dist:
	tar -czvf lab4b-605280083.tar.gz lab4b.c Makefile README test.sh
clean:
	rm -f lab4b lab4b-605280083.tar.gz
