
#include "histogram.h"

#include <cstdio>

int main() {
  Histogram<float> h;

  Histogram<float>::BinningFixed b1(0.0, 1.0, 10.0);
  printf("0.0: %d\n", b1.FindBin(0.0));
  printf("-0.1: %d\n", b1.FindBin(-0.1));
  printf("11.0: %d\n", b1.FindBin(11.0));
  printf("5.0: %d\n", b1.FindBin(5.0));
  printf("10.0: %d\n", b1.FindBin(10.0));

  Histogram<float>::BinningFixed b2(-1.0, 1.0, 1.0);
  printf("0.0: %d\n", b2.FindBin(0.0));
  printf("-1.0: %d\n", b2.FindBin(-1.0));
  printf("-1.1: %d\n", b2.FindBin(-1.1));

  Histogram<float>::BinningFixed b3(0.0, 3.0, 4.0);
  printf("1.0: %d\n", b3.FindBin(1.0));
  printf("3.5: %d\n", b3.FindBin(3.5));
  printf("4.5: %d\n", b3.FindBin(4.5));

  std::vector<float> bins;
  for (unsigned i = 0; i <= 10000; ++i) {
    bins.push_back(i);
  }
  uint32_t a = 0;
  Histogram<float>::BinningDynamic b4(bins);
  for (unsigned j = 0; j < 1000; ++j) {
    for (unsigned i = 0; i < 10000; ++i) {
      a += b4.FindBin(i);
    }
  }
  printf("-1: %d\n", b4.FindBin(-1));
  printf("0: %d\n", b4.FindBin(0));
  printf("3.5: %d\n", b4.FindBin(3.5));
  printf("9999: %d\n", b4.FindBin(9999));
  printf("10000: %d\n", b4.FindBin(10000));
  printf("10001: %d\n", b4.FindBin(10001));

  return 0;
}

