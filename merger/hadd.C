//macro to add histogram files
//NOTE: This macro is kept for back compatibility only.
//Use instead the executable $ROOTSYS/bin/hadd
//
//This macro will add histograms from a list of root files and write them
//to a target root file. The target file is newly created and must not be
//identical to one of the source files.
//
//Author: Sven A. Schmidt, sven.schmidt@cern.ch
//Date:   13.2.2001

//This code is based on the hadd.C example by Rene Brun and Dirk Geppert,
//which had a problem with directories more than one level deep.
//(see macro hadd_old.C for this previous implementation).
//
//The macro from Sven has been enhanced by
//   Anne-Sylvie Nicollerat <Anne-Sylvie.Nicollerat@cern.ch>
// to automatically add Trees (via a chain of trees).
//
//To use this macro, modify the file names in function hadd.
//
//NB: This macro is provided as a tutorial.
//    Use $ROOTSYS/bin/hadd to merge many histogram files



#include <string.h>
#include "TChain.h"
#include "TFile.h"
#include "TMemFile.h"
#include "TH1.h"
#include "THnBase.h"
#include "TList.h"
#include "TTree.h"
#include "TKey.h"
#include "Riostream.h"

#include <cassert>
#include <cstdio>
#include <fcntl.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

void MergeRootfile( TFile *source );

Int_t total_histos;
Int_t total_bins;
Int_t defined_bins;
Int_t total_histos_sparse;
Int_t total_bins_sparse;
Int_t defined_bins_sparse;

int fd_out;
FILE *fout;

static void Usage() {
  printf("usage: hadd <input path> <output path>\n");
}

void hadd(char *input_path, char *output_path);
int main(int argc, char **argv) {
  if (argc < 3) {
    Usage();
    return 1;
  }
  hadd(argv[1], argv[2]);
  return 0;
}

void hadd(char *input_path, char *output_path) {
   // in an interactive ROOT session, edit the file names
   // Target and FileList, then
   // root > .L hadd.C
   // root > hadd()

   //Long64_t bufsize = 1024*1024*64;
   //char *buffer = (char *)malloc(bufsize);
   //FILE *finput = fopen("QAresults_merge.root", "r");
   //Long64_t offset = 0;
   //while (!feof(finput)) {
   //  size_t nbytes = fread(buffer+offset, 1, 4096, finput);
   //  offset += nbytes;
   //}
   //fclose(finput);

   /*struct stat info;
   int retval = stat("/tmp/HFIFO", &info);
   if (retval != 0) {
     retval = mkfifo("/tmp/HFIFO", 0666);
     if (retval != 0)
       abort();
   }*/
   //unlink(output_path);
   if (output_path[0] == '-') {
     fout = stdout;
   } else {
     fd_out = open(output_path, O_WRONLY | O_CREAT, 0644);
     if (fd_out < 0) {
       abort();
     }
     fout = fdopen(fd_out, "w");
     if (!fout) {
       abort();
     }
   }

   total_histos = 0;
   total_bins = 0;
   defined_bins = 0;
   total_histos_sparse = 0;
   total_bins_sparse = 0;
   defined_bins_sparse = 0;

   MergeRootfile( new TFile(input_path) );

   fclose(fout);

   cerr << "Total histos: " << total_histos << endl;
   cerr << "Total bins: " << total_bins << endl;
   cerr << "Defined bins: " << defined_bins << endl;
   cerr << "Total sparse histos: " << total_histos_sparse << endl;
   cerr << "Total sparse bins: " << total_bins_sparse << endl;
   cerr << "Defined sparse bins: " << defined_bins_sparse << endl;
}

string MkName(const TObject *o) {
  TString result = gDirectory->GetPath();
  result.Append("/");
  result.Append(o->GetName());
  return std::string(result.Data(), result.Length());
}

void PPrintName(const TObject *o) {
  string name = MkName(o);
  uint16_t len = name.length();
  fwrite(&len, sizeof(len), 1, fout);
  fwrite(name.data(), len, 1, fout);
}

void PPrintBin(const int64_t bin, const double value) {
  fwrite(&bin, sizeof(bin), 1, fout);
  fwrite(&value, sizeof(value), 1, fout);
  if (bin == -1)
    return;
  int64_t errbin = -32-bin;
  double errval = value*value;
  fwrite(&errbin, sizeof(errbin), 1, fout);
  fwrite(&errval, sizeof(errval), 1, fout);
}

void ProcessSparse(const THnBase *h) {
  total_histos_sparse++;
  total_bins_sparse += h->GetNbins();
  //PPrintName(h);
  //uint16_t dim = h->GetNdimensions();
}

void ProcessHisto(const TH1 *h) {
  total_histos++;
  Int_t nx = h->GetNbinsX();
  Int_t ny = h->GetNbinsY();
  Int_t nz = h->GetNbinsZ();
  assert(nx > 0);
  assert(ny > 0);
  assert(nz > 0);
  defined_bins += nx*ny*nz;
  cerr << "    This histogram has bins " << nx << ", " << ny << ", " << nz << endl;

  PPrintName(h);

  Double_t all_sum = 0.0;
  for (Int_t x = 0; x <= nx; ++x) {
    if (ny > 1) {
      for (Int_t y = 0; y < ny; ++y) {
        if (nz > 1) {
          for (Int_t z = 0; z < nz; ++z) {
            Double_t this_bin = h->GetBinContent(x, y, z);
            if (this_bin > 0) {
              all_sum += this_bin;
              total_bins++;
              PPrintBin((nx+1)*(ny+1)*z + (nx+1)*y + x, this_bin);
            }
          }
        } else {
          Double_t this_bin = h->GetBinContent(x, y);
          if (this_bin > 0) {
            all_sum += this_bin;
            total_bins++;
            PPrintBin((nx+1)*y + x, this_bin);
          }
        }
      }
    } else {
      Double_t this_bin = h->GetBinContent(x);
      if (this_bin > 0) {
        total_bins++;
        all_sum += this_bin;
        PPrintBin(x, this_bin);
      }
    }
  }
  PPrintBin(-1, 0.0);
  cerr << "      All_sums " << all_sum << endl;
}

void MergeRootfile( TFile *source ) {
   TString path(gDirectory->GetPath());
   source->cd( path );
   cerr << "Browsing " << path << endl;

   //gain time, do not add the objects in the list in memory
   TH1::AddDirectory(kFALSE);

   // loop over all keys in this directory
   TIter nextkey( gDirectory->GetListOfKeys() );
   TKey *key, *oldkey=0;
   while ( (key = (TKey*)nextkey())) {

      //keep only the highest cycle number for each key
      if (oldkey && !strcmp(oldkey->GetName(),key->GetName())) continue;

      // read object from first source file
      TObject *obj = key->ReadObj();

      cerr << "NAME IS " << obj->GetName() << endl;

      if ( obj->IsA()->InheritsFrom( TH1::Class() ) ) {
         cerr << "   HELLO, we have the histogram " << obj->GetName() << endl;
         TH1 *h = (TH1*)obj;
         ProcessHisto(h);
      } else if ( obj->IsA()->InheritsFrom( THnBase::Class() )) {
         cerr << "   HELLO-SPARSE " << obj->GetName() << endl;
         THnBase *h = (THnBase*)obj;
         ProcessSparse(h);
      } else if ( obj->IsA()->InheritsFrom( TDirectory::Class() ) ) {
         // it's a subdirectory

         //cerr << "Found subdirectory " << obj->GetName() << endl;
	 source->cd( path + "/" + obj->GetName() );
         MergeRootfile(source);
         source->cd( path );
      } else if ( obj->IsA()->InheritsFrom( TCollection::Class() )) {
        TCollection *coll = (TCollection *)obj;
        //cerr << "List of something in " << obj->GetName() << endl;
        TIter nextelem(coll);
        TObject *elem;
        while ((elem = nextelem())) {
          if (elem->IsA()->InheritsFrom( TH1::Class() )) {
            cerr << "   HELLO, we have the histogram " << elem->GetName() << endl;
            TH1 *h = (TH1 *)elem;
            ProcessHisto(h);
          } else if (elem->IsA()->InheritsFrom( THnBase::Class() )) {
            cerr << "   HELLO-SPARSE " << elem->GetName() << endl;
            THnBase *h = (THnBase *)elem;
            ProcessSparse(h);
          }
        }
      } else {

         // object is of no type that we know or can handle
         cerr << "Unknown object type, name: "
           << obj->GetName() << " title: " << obj->GetTitle() << endl;
      }
      delete obj;

   } // while ( ( TKey *key = (TKey*)nextkey() ) )
}
