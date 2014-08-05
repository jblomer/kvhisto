
OPTFLAGS= -O3 -g
CFLAGS = -Wall -std=c++11
LDFLAGS = -L. -lm

HIST_SOURCES = histogram.h channel.h coord.h binning.h prng.h kvincr.h
KVINCR_SOURCES = kvincr.h kvincr.cc

all: histogram replay libkvincr.so

histogram: histogram.cc $(HIST_SOURCES) libkvincr.so
	$(CXX) $(OPTFLAGS) $(CFLAGS) -o histogram histogram.cc $(LDFLAGS) -lkvincr

replay: replay.cc $(HIST_SOURCES) libkvincr.so
	$(CXX) $(OPTFLAGS) $(CFLAGS) $(LDFLAGS) -o replay replay.cc -lkvincr
	
libkvincr.so: $(KVINCR_SOURCES)
	$(CXX) -fPIC $(OPTFLAGS) $(CFLAGS) -c kvincr.cc
	$(CXX) -shared -Wl,-soname,kvincr.so -o libkvincr.so kvincr.o

clean:
	rm -f histogram replay kvincr.a libkvincr.so
	rm -rf *.dSYM
	rm -f *.o

