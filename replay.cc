
#include "histogram.h"

#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

static bool GetLineFile(FILE *f, string *line) {
  int retval;
  line->clear();
  while ((retval = fgetc(f)) != EOF) {
    char c = retval;
    if (c == '\n')
      break;
    line->push_back(c);
  }
  return (retval != EOF) || !line->empty();
}

vector<string> SplitString(const string &str,
                           const char delim,
                           const unsigned max_chunks = 0) {
  vector<string> result;

  // edge case... one chunk is always the whole string
  if (1 == max_chunks) {
    result.push_back(str);
    return result;
  }

  // split the string
  const unsigned size = str.size();
  unsigned marker = 0;
  unsigned chunks = 1;
  unsigned i;
  for (i = 0; i < size; ++i) {
    if (str[i] == delim) {
      result.push_back(str.substr(marker, i-marker));
      marker = i+1;

      // we got what we want... good bye
      if (++chunks == max_chunks)
        break;
    }
  }

  // push the remainings of the string and return
  result.push_back(str.substr(marker));
  return result;
}

struct HistoDescription {
  HistoDescription() : dimension(0), h(nullptr) { }
  HistoDescription(unsigned d, Histogram<double, uint32_t, ChannelStoreSimple<uint32_t>> *h) : dimension(d), h(h) { }
  unsigned dimension;
  Histogram<double, uint32_t, ChannelStoreSimple<uint32_t>> *h;
};

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s <trace file>\n", argv[0]);
    return 1;
  }
  FILE *ftrace = fopen(argv[1], "r");
  if (!ftrace) return 1;
  
  unordered_map<string, HistoDescription> histos;  
  
  string line;
  uint64_t total_fills = 0;
  uint64_t total_bins = 0;
  while (GetLineFile(ftrace, &line)) {
    if (line.empty()) continue;
    vector<string> tokens = SplitString(line, ' ');
    if (line[0] == 'C') {
      string key = tokens[1];
      unsigned dimension;
      sscanf(tokens[2].c_str(), "%u", &dimension);
      printf("new histo %s dimension %u\n", key.c_str(), dimension);
      
      vector<Binning<double>> binnings;
      unsigned token_pos = 3;
      vector<unsigned> num_bins;
      for (unsigned i = 1; i <= dimension; ++i) {
        unsigned this_num_bins;
        sscanf(tokens[token_pos++].c_str(), "%u", &this_num_bins);
        num_bins.push_back(this_num_bins);
        printf("  new dimension with %u bins\n", this_num_bins);
      }
      
      for (unsigned i = 1; i <= dimension; ++i) {
        std::vector<double> borders;
        borders.reserve(num_bins[i-1]);
        for (unsigned j = 0; j < num_bins[i-1]; ++j) {
          double this_border;
          sscanf(tokens[token_pos++].c_str(), "%lf", &this_border);
          borders.push_back(this_border);
        }
        binnings.push_back(Binning<double>(borders));
      }
      
      Histogram<double, uint32_t, ChannelStoreSimple<uint32_t>> *h;
      h = new Histogram<double, uint32_t, ChannelStoreSimple<uint32_t>>(binnings);
      total_bins += h->size();
      histos[key] = HistoDescription(dimension, h);
    } else if (line[0] == 'F') {
      string key = tokens[1];
      if (histos.find(key) == histos.end()) continue;
      HistoDescription hd = histos[key];
      vector<double> coordinates;
      for (unsigned i = 2; i < hd.dimension+2; ++i) {
        double this_axis;
        sscanf(tokens[i].c_str(), "%lf", &this_axis);
        coordinates.push_back(this_axis);
      }
      //printf("dimension is %u, coord size is %lu\n", hd.dimension, coordinates.size());
      hd.h->Fill(coordinates);     
      total_fills++;
    } else {
      abort();
    }
  }
  
  uint64_t filled_bins = 0;
  for (auto i : histos) {
    filled_bins += i.second.h->occupied();
  }
  uint64_t sum = 0;
  for (auto i : histos) {
    sum += i.second.h->sum();
  }
  
  printf("Replayed %lu histograms\n", histos.size());
  printf("Replayed %llu fillings\n", total_fills);
  printf("Replayed %llu defined bins\n", total_bins);
  printf("Replayed %llu filled bins\n", filled_bins);
  printf("Replayed %llu sum\n", sum);
  
  
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

