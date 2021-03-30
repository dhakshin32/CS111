#NAME: Dhakshin Suriakannu
#EMAIL: bruindhakshin@g.ucla.edu
#ID: 605280083

default:
	gcc -o lab0 -g -Wall -Wextra lab0.c

check:
	./check.sh

clean:
	rm -f *.o lab0 *.txt *.tar.gz *.txt~ *.c~

dist: default
	tar -zvcf lab0-605280083.tar.gz lab0.c README Makefile check.sh breakpoint.png backtrace.png




