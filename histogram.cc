
#include "histogram.h"
#include "prng.h"

#include <cstdio>

int main() {
  //std::array<int, 1> a0{ 1 };
  //for (auto i : a0) {
  //  printf("%d\n", i);
  //}

  //Binning<double> b1(1, 1, 100);
  //Binning<double> b2(1, 1, 100);
  //Histogram<double, uint32_t, ChannelStoreSimple<uint32_t>, 2 > h2({{b1, b2}});
  //h2.Fill(1, 1);


  //Binning<double> bins(1, 1, 100000);
  std::vector<double> borders;
  borders.reserve(100000);
  for (unsigned i = 0; i < 100000; ++i)
    borders.push_back(i);
  Binning<double> bins(borders);
  
  Prng prng;
  prng.InitSeed(42);
  
  Histogram<double, uint32_t, ChannelStoreSparse<uint32_t>> h({{bins}});
  for (unsigned i = 0; i < 100000000; ++i)
      h.Fill({prng.Next(100000) + 0.0});

  printf("Occupied bins %llu, Histogram sum %u\n", h.occupied(), h.sum());

  /*Histogram<double, float, ChannelStoreSimple<float>, 1 > h({{bins}});
  for (unsigned j = 0; j < 1000; ++j) {
    for (unsigned i = 0; i < 100000; ++i) {
      //h.Fill({{static_cast<float>(i)}});
      h.Fill(i);
    }
  }*/

  /*HistogramBase< float, uint32_t, ChannelStoreSimple<float> > h;
  h.SetDimensions(1);
  std::vector<float> bins;
  for (unsigned i = 0; i <= 10000; ++i) {
    bins.push_back(i);
  }
  //h.SetBinning(1, Histogram<float, uint32_t>::Binning(bins));
  h.SetBinning(1, HistogramBase< float, uint32_t, ChannelStoreSimple<float> >::Binning(0, 1, 10000));
  for (unsigned j = 0; j < 10000; ++j) {
    for (unsigned i = 0; i < 10000; ++i) {
      h.Fill(i);
    }
  }*/

  /*Histogram<float>::BinningFixed b1(0.0, 1.0, 10.0);
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
  printf("10001: %d\n", b4.FindBin(10001));*/

  return 0;
}
