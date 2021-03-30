#NAME: Shrea Chari
#EMAIL: shreachari@gmail.com
#ID: 005318456
#NAME: Dhakshin Suriakannu
#EMAIL: dhakshin.s@g.ucla.edu
#ID: 605280083
CC=gcc
flags= -Wall -Wextra -g

default:
	$(CC) $(flags) lab3a.c -o lab3a

clean:
	rm -f lab3a lab3a-605280083.tar.gz

dist: default
	tar -czvf lab3a-605280083.tar.gz Makefile README lab3a.c ext2_fs.h