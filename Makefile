
OPTFLAGS= -O3 -g
CFLAGS = -Wall -std=c++11
LDFLAGS = -lm -L.

HIST_SOURCES = histogram.h channel.h coord.h binning.h prng.h kvincr.h
KVINCR_SOURCES = kvincr.h kvincr.cc

all: histogram replay kvincr.so

histogram: histogram.cc $(HIST_SOURCES) kvincr.so
	$(CXX) $(OPTFLAGS) $(CFLAGS) $(LDFLAGS) -o histogram histogram.cc -lkvincr

replay: replay.cc $(HIST_SOURCES)
	$(CXX) $(OPTFLAGS) $(CFLAGS) $(LDFLAGS) -o replay replay.cc -lkvincr
	
kvincr.so: $(KVINCR_SOURCES)
	$(CXX) -fPIC $(OPTFLAGS) $(CFLAGS) -c kvincr.cc
	$(CXX) -shared kvincr.o

clean:
	rm -f histogram replay kvincr.a kvincr.so
	rm -rf *.dSYM
	rm -f *.o

