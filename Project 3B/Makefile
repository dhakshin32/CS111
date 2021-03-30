#NAME: Shrea Chari
#EMAIL: shreachari@gmail.com
#ID: 005318456
#NAME: Dhakshin Suriakannu
#EMAIL: dhakshin.s@g.ucla.edu
#ID: 605280083

UID=605280083

default:
	ln my_lab3b lab3b
	chmod u+x lab3b

clean:
	rm -f lab3b-$(UID).tar.gz lab3b

dist: default
	tar -czvf lab3b-$(UID).tar.gz Makefile README lab3b.py my_lab3b
