#NAME: Dhakshin Suriakannu
#EMAIL: bruindhakshin@g.ucla.edu
#ID: 605280083

CC = gcc
flags = -lz -Wall -Wextra
default:
	${CC} ${flags} -o lab1b-client lab1b-client.c
	${CC} ${flags} -o lab1b-server lab1b-server.c

client:
	${CC} ${flags} -o lab1b-client lab1b-client.c

server:
	${CC} ${flags} -o lab1b-server lab1b-server.c

dist:
	tar -cvzf lab1b-605280083.tar.gz lab1b-server.c lab1b-client.c README Makefile

clean:
	rm -f lab1b-605280083.tar.gz lab1b-client lab1b-server

