#include <cassert>
#include <cstdio>
#include <inttypes.h>
#include <string>
#include <vector>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <ramcloud/CRamCloud.h>

#include "prng.h"
#include "md5.h"

const unsigned kMaxHName = 1024;
const unsigned kBatchSize = 10000;

struct rc_client *client;
uint64_t global_tblid;
uint32_t nflushes = 0;
double totalSum = 0;

struct JointKey {
  JointKey(const uint32_t logical_tblid, const uint64_t key)
    : logical_tblid(logical_tblid)
    , key(key)
  {}
  uint32_t logical_tblid;
  uint64_t key;
} __attribute__((packed));

void ReadBunch(std::vector<uint32_t> *logical_tblids,
               std::vector<int64_t> *keys)
{
  assert(logical_tblids->size() == keys->size());
  const unsigned N = logical_tblids->size();
  if (N == 0) return;

  printf("READING %u BINS\n", N);
  nflushes++;
  std::vector<JointKey> joint_keys;
  joint_keys.reserve(N);
  for (unsigned i = 0; i < N; ++i) {
    joint_keys.push_back(JointKey((*logical_tblids)[i], (*keys)[i]));
  }

  double *mValues = reinterpret_cast<double *>(malloc(N * sizeof(double)));
  uint32_t *mValSz = reinterpret_cast<uint32_t *>(malloc(N * sizeof(uint32_t)));
  const uint16_t szMultiOpRead = rc_multiOpSizeOf(MULTI_OP_READ);
  unsigned char *mReadObjects = reinterpret_cast<unsigned char *>
      (malloc(N * szMultiOpRead));
  void **pmReadObjects = reinterpret_cast<void **>
      (malloc(N * sizeof(pmReadObjects[0])));

  for (unsigned i = 0; i < N; ++i) {
      pmReadObjects[i] = mReadObjects + (i * szMultiOpRead);
      rc_multiReadCreate(global_tblid,
                         &joint_keys[i], sizeof(JointKey),
                         &mValues[i], sizeof(double), &mValSz[i],
                         pmReadObjects[i]); 
  }
  rc_multiRead(client, pmReadObjects, N);

  for (unsigned i = 0; i < N; ++i) {
    Status thisStatus =
    rc_multiOpStatus(pmReadObjects[i], MULTI_OP_READ);
    if (thisStatus != STATUS_OK) {
      printf("read failure at %u\n", i);
      abort();
    }
    totalSum += mValues[i];
    rc_multiOpDestroy(pmReadObjects[i], MULTI_OP_READ);
  }
  free(pmReadObjects);
  free(mReadObjects);
  free(mValues);
  free(mValSz);

  logical_tblids->clear();
  keys->clear();
}

void Usage(const char *progname) {
    printf("Usage: %s <input file> <table name> [locator]\n",
           progname);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    Usage(argv[0]);
    return 1;
  }

  FILE *fin;
  if (std::string(argv[1]) == "-")
    fin = stdin;
  else
    fin = fopen(argv[1], "r");
  if (!fin) {
    printf("failed to open input file\n");
    abort();
  }
  
  Status status;
  const char *locator = "infrc:host=192.168.1.119,port=11100";
  if (argc >= 4)
      locator = argv[3];
  status = rc_connect(locator, "main", &client);
  if (status != STATUS_OK) {
    printf("failed connecting to RAMCloud\n");
    abort();
  }

  const char *table_name = argv[2];
  status = rc_getTableId(client, table_name, &global_tblid);
  if (status == STATUS_TABLE_DOESNT_EXIST) {
    //printf("table %s exists, stop\n", table_name);
    //abort();
    printf("failed getting global table id\n");
    abort();
  }

  unsigned ntables = 0;
  unsigned nzerotables = 0;
  unsigned totalBins = 0;
  unsigned nzerobins = 0;
  std::vector<uint32_t> logical_tblids;
  std::vector<int64_t> keys;
  while (1) {
    int nbytes;
    uint16_t hname_len;
    char name_buf[kMaxHName+1];

    nbytes = fread(&hname_len, sizeof(hname_len), 1, fin);
    if (nbytes == 0)
      break;

    ntables++;
    if (hname_len > kMaxHName) {
      printf("name too long! (%d)\n", hname_len);
      abort();
    }
    fread(name_buf, hname_len, 1, fin);
    name_buf[hname_len] = '\0';

    // Transform into MD5 hash
    md5_byte_t digest[16];
    md5_state_t pms;
    md5_init(&pms);
    md5_append(&pms, (const md5_byte_t *)name_buf, hname_len);
    md5_finish(&pms, digest);

    char table_name[33];
    for (unsigned i = 0; i < 16; ++i) {
      char dgt1 = (unsigned)digest[i] / 16;
      char dgt2 = (unsigned)digest[i] % 16;
      dgt1 += (dgt1 <= 9) ? '0' : 'a' - 10;
      dgt2 += (dgt2 <= 9) ? '0' : 'a' - 10;
      table_name[i*2] = dgt1;
      table_name[i*2+1] = dgt2;
    }
    table_name[32] = '\0';

    printf("Reading histogram %s (%s)\n", table_name, name_buf);

    unsigned nbins = 0;
    while (1) {
      int64_t bin;
      fread(&bin, sizeof(bin), 1, fin);
      if (bin == -1)
        break;

      logical_tblids.push_back(ntables);
      keys.push_back(bin);
      nbins++;
      totalBins++;

      if (keys.size() == kBatchSize) {
        ReadBunch(&logical_tblids, &keys);
      }
      if (totalBins % 500000 == 0) {
        printf(" --- TOTAL: %u bins --- \n", totalBins);
      }
    }
    printf("... %u bins\n", nbins);
    if (nbins == 0) {
      nzerotables++;
    }
    //if (mode == 'E') {
    //
    //}
  }
  ReadBunch(&logical_tblids, &keys);

  printf("finished reading (%u tables / %u non-empty, %u bins / %u non-empty)\n",
         ntables, ntables-nzerotables, totalBins, totalBins-nzerobins);
  printf("total sum: %lf\n", totalSum);
  return 0;
}
