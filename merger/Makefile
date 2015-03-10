all: merger create_table

DEBUG = no
ifeq ($(DEBUG),yes)
DEBUGFLAGS = -DDEBUG
endif

md5.o: md5.c
	gcc -O2 -g -Wall -c md5.c

create_table: create_table.cc
	g++ -std=c++0x -pthread -O2 -g -Wall -o create_table create_table.cc -lramcloud

merger: merger.cc prng.h md5.h md5.o
	g++ -std=c++0x -pthread -O2 -g -Wall -o merger merger.cc md5.o -lramcloud

enum: enum.cc
	g++ -std=c++0x -pthread -O2 -g -Wall -o enum enum.cc -lramcloud


clean:
	rm -f *.o *.d
	rm -f merger enum
