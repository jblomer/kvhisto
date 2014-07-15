
OPTFLAGS= -O3 -g
CFLAGS = -Wall -std=c++11
LDFLAGS = -lm

HIST_SOURCES = histogram.h channel.h coord.h binning.h

all: histogram replay

histogram: histogram.cc $(HIST_SOURCES)
	$(CXX) $(OPTFLAGS) $(CFLAGS) $(LDFLAGS) -o histogram histogram.cc

replay: replay.cc $(HIST_SOURCES)
	$(CXX) $(OPTFLAGS) $(CFLAGS) $(LDFLAGS) -o replay replay.cc

clean:
	rm -f histogram 

