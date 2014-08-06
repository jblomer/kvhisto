
OPTFLAGS= -O3 -g
CFLAGS = -Wall -std=c++0x
LDFLAGS = -L. -L../ramcloud/obj.increment-double -lm

RC_INCLUDE = -I../ramcloud/src -I../ramcloud/obj.master -I../ramcloud/gtest/include

HIST_SOURCES = histogram.h channel.h coord.h binning.h prng.h kvincr.h
KVINCR_SOURCES = kvincr.h kvincr.cc

all: histogram libkvincr.so

histogram: histogram.cc $(HIST_SOURCES) libkvincr.so
	$(CXX) $(OPTFLAGS) $(CFLAGS) -o histogram histogram.cc $(LDFLAGS) -lkvincr -lramcloud

replay: replay.cc $(HIST_SOURCES) libkvincr.so
	$(CXX) $(OPTFLAGS) $(CFLAGS) $(LDFLAGS) -o replay replay.cc -lkvincr -lramcloud
	
libkvincr.so: $(KVINCR_SOURCES)
	$(CXX) -fPIC $(RC_INCLUDE) $(OPTFLAGS) $(CFLAGS) -c kvincr.cc
	$(CXX) $(LDFLAGS) -shared -Wl,-soname,libkvincr.so -o libkvincr.so kvincr.o

clean:
	rm -f histogram replay kvincr.a libkvincr.so
	rm -rf *.dSYM
	rm -f *.o

