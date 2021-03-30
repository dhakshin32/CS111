#NAME: Dhakshin Suriakannu
#EMAIL: bruindhakshin@g.ucla.edu
#ID: 605280083

default: 
	gcc -o lab4c_tcp -Wall -Wextra -lm -lmraa -g lab4c_tcp.c
	gcc -o lab4c_tls -Wall -Wextra -lm -lmraa -g -lssl -lcrypto lab4c_tls.c

clean:
	rm -f lab4c-605280083.tar.gz lab4c_tcp lab4c_tls

dist:
	tar -czvf lab4c-605280083.tar.gz README Makefile lab4c_tls.c lab4c_tcp.c