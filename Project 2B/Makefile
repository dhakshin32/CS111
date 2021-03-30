#NAME: Dhakshin Suriakannu
#EMAIL: bruindhakshin@g.ucla.edu
#ID: 605280083
CC=gcc
flags= -Wall -Wextra -pthread -ggdb3

default:
	$(CC) $(flags) -g -o lab2_list -lpthread lab2_list.c SortedList.c

tests: default
	rm -f lab2_list.csv
	./lab2_list --threads=1  --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=m	>> lab2b_list.csv
	./lab2_list --threads=12  --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16  --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=24  --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12  --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16  --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=24  --iterations=1000 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=1 --yield=id --lists=4 --iterations=1 >> lab2b_list.csv
	-./lab2_list --threads=1 --yield=id --lists=4 --iterations=2 >> lab2b_list.csv
	-./lab2_list --threads=1 --yield=id --lists=4 --iterations=4 >> lab2b_list.csv
	-./lab2_list --threads=1 --yield=id --lists=4 --iterations=8 >> lab2b_list.csv
	-./lab2_list --threads=1 --yield=id --lists=4 --iterations=16 >> lab2b_list.csv
	-./lab2_list --threads=12 --yield=id --lists=4 --iterations=1 >> lab2b_list.csv
	-./lab2_list --threads=12 --yield=id --lists=4 --iterations=2 >> lab2b_list.csv
	-./lab2_list --threads=12 --yield=id --lists=4 --iterations=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --yield=id --lists=4 --iterations=8 >> lab2b_list.csv
	-./lab2_list --threads=12 --yield=id --lists=4 --iterations=16 >> lab2b_list.csv
	-./lab2_list --threads=4 --yield=id --lists=4 --iterations=1 >> lab2b_list.csv
	-./lab2_list --threads=4 --yield=id --lists=4 --iterations=2 >> lab2b_list.csv
	-./lab2_list --threads=4 --yield=id --lists=4 --iterations=4 >> lab2b_list.csv
	-./lab2_list --threads=4 --yield=id --lists=4 --iterations=8 >> lab2b_list.csv
	-./lab2_list --threads=4 --yield=id --lists=4 --iterations=16 >> lab2b_list.csv
	-./lab2_list --threads=8 --yield=id --lists=4 --iterations=1 >> lab2b_list.csv
	-./lab2_list --threads=8 --yield=id --lists=4 --iterations=2 >> lab2b_list.csv
	-./lab2_list --threads=8 --yield=id --lists=4 --iterations=4 >> lab2b_list.csv
	-./lab2_list --threads=8 --yield=id --lists=4 --iterations=8 >> lab2b_list.csv
	-./lab2_list --threads=8 --yield=id --lists=4 --iterations=16 >> lab2b_list.csv
	-./lab2_list --threads=16 --yield=id --lists=4 --iterations=1 >> lab2b_list.csv
	-./lab2_list --threads=16 --yield=id --lists=4 --iterations=2 >> lab2b_list.csv
	-./lab2_list --threads=16 --yield=id --lists=4 --iterations=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --yield=id --lists=4 --iterations=8 >> lab2b_list.csv
	-./lab2_list --threads=16 --yield=id --lists=4 --iterations=16 >> lab2b_list.csvprofile
	./lab2_list --threads=1 --yield=id --lists=4 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --yield=id --lists=4 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --yield=id --lists=4 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --yield=id --lists=4 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --yield=id --lists=4 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --yield=id --lists=4 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --yield=id --lists=4 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --yield=id --lists=4 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --yield=id --lists=4 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --yield=id --lists=4 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --yield=id --lists=4 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --yield=id --lists=4 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --yield=id --lists=4 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --yield=id --lists=4 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --yield=id --lists=4 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --yield=id --lists=4 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --yield=id --lists=4 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --yield=id --lists=4 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --yield=id --lists=4 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --yield=id --lists=4 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --yield=id --lists=4 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --yield=id --lists=4 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --yield=id --lists=4 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --yield=id --lists=4 --iterations=80 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --yield=id --lists=4 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --yield=id --lists=4 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --yield=id --lists=4 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --yield=id --lists=4 --iterations=80 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --yield=id --lists=4 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --yield=id --lists=4 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --yield=id --lists=4 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --yield=id --lists=4 --iterations=80 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --yield=id --lists=4 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --yield=id --lists=4 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --yield=id --lists=4 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --yield=id --lists=4 --iterations=80 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --yield=id --lists=4 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --yield=id --lists=4 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --yield=id --lists=4 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --yield=id --lists=4 --iterations=80 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000 --sync=m	--lists=4 >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=m	--lists=4 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=m	--lists=4 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=m	--lists=4 >> lab2b_list.csv
	./lab2_list --threads=12  --iterations=1000 --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000 --sync=s	--lists=4 >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=12  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=m	--lists=8 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
	./lab2_list --threads=12  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000 --sync=s	--lists=8 >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
	./lab2_list --threads=12  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=m	--lists=16 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=m	--lists=16 >> lab2b_list.csv
	./lab2_list --threads=12  --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000 --sync=s	 --lists=16 >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=s	 --lists=16 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=s  --lists=16 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=s  --lists=16 >> lab2b_list.csv
	./lab2_list --threads=12  --iterations=1000 --sync=s  --lists=16 >> lab2b_list.csv	
graphs: tests
		./lab2b_list.gp
profile: default
	rm -f ./raw.gperf profile.out
	LD_PRELOAD=/usr/lib64/libprofiler.so \
	CPUPROFILE=./raw.gperf ./lab2_list --iterations=1000 --threads=12 --sync=s
	/usr/bin/pprof --text ./lab2_list ./raw.gperf > profile.out
	/usr/bin/pprof --list=thread_worker ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf 
dist: graphs profile
	tar -czvf lab2b-605280083.tar.gz SortedList.h SortedList.c lab2_list.c Makefile README \
		lab2b_list.csv profile.out lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png \
		lab2b_5.png lab2b_list.gp
clean:
	rm -f lab2_list lab2b-605280083.tar.gz