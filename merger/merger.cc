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

struct JointKey {
  JointKey(const uint32_t logical_tblid, const uint64_t key)
    : logical_tblid(logical_tblid)
    , key(key)
  {}
  uint32_t logical_tblid;
  uint64_t key;
} __attribute__((packed));

void FlushBuffer(std::vector<uint32_t> *logical_tblids,
                 std::vector<int64_t> *keys,
                 std::vector<double> *values,
                 const bool dry_run)
{
  assert((logical_tblids->size() == keys->size()) &&
         (keys->size() == values->size()));
  const unsigned N = logical_tblids->size();
  if (N == 0) return;

  Prng prng;
  prng.InitLocaltime();
  uint32_t *shuffled = reinterpret_cast<uint32_t *>
    (malloc(N * sizeof(uint32_t)));
  // Init with identity
  for (unsigned i = 0; i < N; ++i)
    shuffled[i] = i;
  // Shuffle (no shuffling for the last element)
  for (unsigned i = 0; i < N-1; ++i) {
    const uint32_t swap_idx = i + prng.Next(N - i);
    uint32_t tmp = shuffled[i];
    shuffled[i] = shuffled[swap_idx];
    shuffled[swap_idx]  = tmp;
  }

  printf("INCREMENTING %u BINS\n", N);
  std::vector<JointKey> joint_keys;
  joint_keys.reserve(N);
  for (unsigned i = 0; i < N; ++i) {
    joint_keys.push_back(JointKey((*logical_tblids)[i], (*keys)[i]));
  }

  const uint16_t szMultiOpIncrement = rc_multiOpSizeOf(MULTI_OP_INCREMENT);
  unsigned char *mIncrementObjects = reinterpret_cast<unsigned char *>
      (malloc(N * szMultiOpIncrement));
  void **pmIncrementObjects = reinterpret_cast<void **>
      (malloc(N * sizeof(pmIncrementObjects[0])));

  for (unsigned i = 0; i < N; ++i) {
      pmIncrementObjects[i] = mIncrementObjects + (i * szMultiOpIncrement);
      rc_multiIncrementCreate(global_tblid,
                              &joint_keys[shuffled[i]], sizeof(JointKey),
                              0, (*values)[shuffled[i]],
                              NULL, pmIncrementObjects[i]);
  }
  if (!dry_run) {
    rc_multiIncrement(client, pmIncrementObjects, N);

    for (unsigned i = 0; i < N; ++i) {
      Status thisStatus =
        rc_multiOpStatus(pmIncrementObjects[i], MULTI_OP_INCREMENT);
      if (thisStatus != STATUS_OK) {
        printf("upload failure at %u\n", i);
        abort();
      }
      rc_multiOpDestroy(pmIncrementObjects[i], MULTI_OP_INCREMENT);
    }
  }
  free(shuffled);
  free(pmIncrementObjects);
  free(mIncrementObjects);

  logical_tblids->clear();
  keys->clear();
  values->clear();
}

void Usage(const char *progname) {
    printf("Usage: %s <input file> <table name> <num of workers> [locator]\n",
           progname);
}

int main(int argc, char **argv) {
  if (argc < 4) {
    Usage(argv[0]);
    return 1;
  }

  char mode = argv[1][0];
  char *input = &(argv[1][1]);
  printf("Using file %s, mode %c\n", input, mode);
  FILE *fin;
  if (input[0] == '-') {
    printf("using stdin\n");
    fin = stdin;
  } else {
    fin = fopen(input, "r");
  }

  if (mode != 'D') {
    Status status;
    const char *locator = "infrc:host=192.168.1.119,port=11100";
    if (argc >= 5)
      locator = argv[4];
    status = rc_connect(locator, "main", &client);
    if (status != STATUS_OK) {
      printf("failed connecting to RAMCloud\n");
      abort();
    }

    const char *table_name = argv[2];
    const uint32_t num_nodes = atoi(argv[3]);
    status = rc_getTableId(client, table_name, &global_tblid);
    if (status == STATUS_TABLE_DOESNT_EXIST) {
      //printf("table %s exists, stop\n", table_name);
      //abort();
      status = rc_createTable(client, table_name, num_nodes);
      if (status != STATUS_OK) {
        printf("failed creating table %s\n", table_name);
        abort();
      }
      status = rc_getTableId(client, table_name, &global_tblid);
      if (status != STATUS_OK) {
        printf("failed getting global table id\n");
        abort();
      }
    }
  }

  unsigned ntables = 0;
  unsigned nzerotables = 0;
  unsigned totalBins = 0;
  std::vector<uint32_t> logical_tblids;
  std::vector<int64_t> keys;
  std::vector<double> values;
  double total_sum = 0.0;
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

    if (mode == 'I') {
      printf("Merging histogram %s (%s)\n", table_name, name_buf);
    } else if (mode == 'D') {
      printf("Dry-run merging histogram %s (%s)\n", table_name, name_buf);
    }

    unsigned nbins = 0;
    while (1) {
      struct {
        int64_t bin;
        double value;
      } item;
      fread(&item, sizeof(item), 1, fin);
      if (item.bin == -1)
        break;

      if ((mode == 'I') || (mode == 'D')) {
        logical_tblids.push_back(ntables);
        keys.push_back(item.bin);
        values.push_back(item.value);
        total_sum += item.value;
      }
      nbins++;
      totalBins++;

      if (keys.size() == kBatchSize) {
        FlushBuffer(&logical_tblids, &keys, &values, mode == 'D');
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
  FlushBuffer(&logical_tblids, &keys, &values, mode == 'D');

  printf("finished reading (%u tables / %u non-empty, %u bins)\n",
         ntables, ntables-nzerotables, totalBins);
  printf("Total sum: %lf\n", total_sum);
  return 0;
}
