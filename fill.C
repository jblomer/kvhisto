#include <TH1F.h>
#include <TVector.h>

void fill() {
  //gROOT->SetStyle("Plain");
  // gStyle->SetOptStat(0);
  // gStyle->SetPalette(1);
  
  unsigned N = 100000;
  unsigned i;
  Double_t bins[N];
  for (i = 0; i < N; ++i)
    bins[i] = i;
  
  
  TH1F *h = new TH1F();
  h->SetBins(N-10, bins);
  //TH1F *h = new TH1F(borders);
  for (int j = 0; j < 1000; ++j) {
    for (i = 0; i < 100000; ++i)
      h->Fill(i);
  }
  //new TCanvas;
  //h->Draw();
}
