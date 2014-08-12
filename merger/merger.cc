#include <cassert>
#include <cstdio>
#include <inttypes.h>
#include <string>
#include <vector>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "prng.h"
#include "CRamCloud.h"
#include "md5.h"

const unsigned kMaxHName = 1024;
const unsigned kNumNodes = 1;
const unsigned kBatchSize = 1000;

struct rc_client *client;

void FlushBuffer(std::vector<uint64_t> *tblids,
                 std::vector<int64_t> *keys, 
                 std::vector<double> *values)
{
  assert((tblids->size() == keys->size()) && (keys->size() == values->size()));
  const unsigned N = tblids->size();
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
  const uint16_t szMultiOpIncrement = rc_multiOpSizeOf(MULTI_OP_INCREMENT);
  unsigned char *mIncrementObjects = reinterpret_cast<unsigned char *>
      (malloc(N * szMultiOpIncrement));
  void **pmIncrementObjects = reinterpret_cast<void **>
      (malloc(N * sizeof(pmIncrementObjects[0])));
  for (unsigned i = 0; i < N; ++i) {
      pmIncrementObjects[i] = mIncrementObjects + (i * szMultiOpIncrement);
      rc_multiIncrementCreate(tblids->at(shuffled[i]), 
                              &keys->at(shuffled[i]), sizeof(int64_t),
                              0, values->at(shuffled[i]), 
                              NULL, pmIncrementObjects[i]);
  }
  rc_multiIncrement(client, pmIncrementObjects, N);
  
  free(shuffled);
  
  for (unsigned i = 0; i < N; ++i) {
      Status thisStatus = 
          rc_multiOpStatus(pmIncrementObjects[i], MULTI_OP_INCREMENT);
      if (thisStatus != STATUS_OK) {
        printf("upload failure at %u\n", i);
        abort();
      }
      rc_multiOpDestroy(pmIncrementObjects[i], MULTI_OP_INCREMENT);
  }
  free(pmIncrementObjects);
  free(mIncrementObjects);
  
  tblids->clear();
  keys->clear();
  values->clear();
}

int main(int argc, char **argv) {
  Status status;
  const char *locator = "infrc:host=192.168.1.119,port=11100";
  if (argc >= 3)
    locator = argv[2];
  status = rc_connect(locator, "main", &client);
  if (status != STATUS_OK) {
    printf("failed connecting to RAMCloud\n");
    abort();
  }
  
  char mode = argv[1][0];
  char *input = &(argv[1][1]);
  printf("Using file %s\n", input);
  int fd_input = open(input, O_RDONLY);

  unsigned ntables = 0;
  std::vector<uint64_t> tblids;
  std::vector<int64_t> keys;
  std::vector<double> values;
  while (1) {
    int nbytes;
    uint16_t hname_len;
    char name_buf[kMaxHName+1];
    
    nbytes = read(fd_input, &hname_len, sizeof(hname_len));
    if (nbytes <= 0)
      break;

    ntables++;
    if (hname_len > kMaxHName) {
      printf("name to long!\n");
      abort();
    }
    read(fd_input, name_buf, hname_len);
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
    
    if (mode == 'C') {
      printf("Creating table %s for histogram %s\n", table_name, name_buf);
      status = rc_createTable(client, table_name, kNumNodes);
      if (status != STATUS_OK) {
        printf("failed creating table\n");
        abort();
      }
    }
    uint64_t tblid;
    if (mode == 'I') {
      status = rc_getTableId(client, table_name, &tblid);
      if (status != STATUS_OK) {
        printf("failed to get table id\n");
        abort();
      }
      printf("Table id for table name %s is %lu (%s)\n", table_name, tblid ,name_buf);
    }
    
    unsigned nbins = 0;
    while (1) {
      struct {
        int64_t bin;
        double value;
      } item;
      read(fd_input, &item, sizeof(item));
      if (item.bin == -1)
        break;
      
      if (mode == 'I') {
        tblids.push_back(tblid);
        keys.push_back(item.bin);
        values.push_back(item.value);
      }
      nbins++;
      
      if (keys.size() == kBatchSize) FlushBuffer(&tblids, &keys, &values);
    }
    printf("... %u bins\n", nbins);
  }
  FlushBuffer(&tblids, &keys, &values);
  
  printf("finished reading (%u tables)\n", ntables);
  return 0;
}