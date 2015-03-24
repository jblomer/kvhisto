all: merger create_table hadd hbins readout readout_multi

DEBUG = no
ifeq ($(DEBUG),yes)
DEBUGFLAGS = -DDEBUG
endif

ROOT_INCLUDES = -I $(shell root-config --incdir)
ROOT_LIBS =  -L/usr/lib64/root -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix  -lMathCore -lThread -pthread -lm -ldl -rdynamic

md5.o: md5.c
	gcc -O2 -g -Wall -c md5.c

hadd: hadd.C
	g++ -O2 $(ROOT_INCLUDES) $(ROOT_LIBS) -o hadd hadd.C 

hbins: hbins.C
	g++ -O2 $(ROOT_INCLUDES) $(ROOT_LIBS) -o hbins hbins.C

create_table: create_table.cc
	g++ -std=c++0x -pthread -O2 -g -Wall -o create_table create_table.cc -lramcloud

merger: merger.cc prng.h md5.h md5.o
	g++ -std=c++0x -pthread -O2 -g -Wall -o merger merger.cc md5.o -lramcloud

enum: enum.cc
	g++ -std=c++0x -pthread -O2 -g -Wall -o enum enum.cc -lramcloud

readout: readout.cc
	g++ -std=c++0x -pthread -O2 -g -Wall -o readout readout.cc -lramcloud

readout_multi: readout_multi.cc md5.h md5.o
	g++ -std=c++0x -pthread -O2 -g -Wall -o readout_multi readout_multi.cc md5.o -lramcloud

clean:
	rm -f *.o *.d
	rm -f merger enum hadd create_table
