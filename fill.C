#include <TH1F.h>

void fill() {
  TH1F *h = new TH1F("fStep", "fStep", 1, 1, 100000);
  for (int j = 0; j < 10000; ++j)
    for (int i = 0; i < 100000; ++i)
      h->Fill(i);
 }
