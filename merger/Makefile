INCLUDE_PATH = -I../../ramcloud/src -I../../ramcloud/obj.increment-double -I../../ramcloud/gtest/include
LD_PATH = ../../ramcloud/obj.increment-double

all: merger enum

DEBUG = no
ifeq ($(DEBUG),yes)
DEBUGFLAGS = -DDEBUG
endif

md5.o: md5.c
	gcc -O2 -g -Wall -c md5.c

merger: merger.cc prng.h md5.h md5.o
	g++ -std=c++0x -L$(LD_PATH) $(INCLUDE_PATH) -pthread -O2 -g -Wall -o merger merger.cc md5.o -lramcloud

enum: enum.cc
	g++ -std=c++0x -L$(LD_PATH) $(INCLUDE_PATH) -pthread -O2 -g -Wall -o enum enum.cc -lramcloud


clean:
	rm -f *.o *.d
	rm -f merger enum