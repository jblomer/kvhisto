
OPTFLAGS= -O3 -g
CFLAGS = -Wall -std=c++11
LDFLAGS = -lm

HIST_SOURCES = histogram.h channel.h coord.h binning.h prng.h kvincr.h
KVINCR_SOURCES = kvincr.h kvincr.cc

all: histogram replay kvincr.a

histogram: histogram.cc $(HIST_SOURCES)
	$(CXX) $(OPTFLAGS) $(CFLAGS) $(LDFLAGS) -o histogram histogram.cc

replay: replay.cc $(HIST_SOURCES)
	$(CXX) $(OPTFLAGS) $(CFLAGS) $(LDFLAGS) -o replay replay.cc
	
kvincr.a: $(KVINCR_SOURCES)
	$(CXX) $(OPTFLAGS) $(CFLAGS) -c kvincr.cc
	ar -cvq kvincr.a kvincr.o

clean:
	rm -f histogram replay kvincr.a
	rm -rf *.dSYM
	rm -f *.o

