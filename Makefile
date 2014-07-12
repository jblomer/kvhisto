
OPTFLAGS= -O3 -g
CFLAGS = -Wall -std=c++11
LDFLAGS = -lm

all: histogram

histogram: histogram.cc histogram.h channel.h
	$(CXX) $(OPTFLAGS) $(CFLAGS) $(LDFLAGS) -o histogram histogram.cc

clean:
	rm -f histogram 

