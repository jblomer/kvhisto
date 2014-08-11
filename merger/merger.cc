
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

int main(int argc, char **argv) {
  struct rc_client *client;
  Status status;
  const char *locator = "infrc:host=192.168.1.119,port=11100";
  if (argc >= 3)
    locator = argv[2];
  status = rc_connect(locator, "main", &client);
  if (status != STATUS_OK) {
    printf("failed connecting to RAMCloud\n");
    abort();
  }
  
  char *input = argv[1];
  int fd_input = open(input, O_RDONLY);
  
  unsigned ntables = 0;
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
    
    printf("Creating table %s for histogram %s\n", table_name, name_buf);
    status = rc_createTable(client, table_name, kNumNodes);
    if (status != STATUS_OK) {
      printf("failed creating table\n");
      abort();
    }
    uint64_t tblid;
    status = rc_getTableId(client, table_name, &tblid);
    if (status != STATUS_OK) {
      printf("failed to get table id\n");
      abort();
    }
    
    std::vector<int64_t> keys;
    std::vector<double> values;
    while (1) {
      struct {
        int64_t bin;
        double value;
      } item;
      read(fd_input, &item, sizeof(item));
      if (item.bin == -1)
        break;
      keys.push_back(item.bin);
      values.push_back(item.value);
    }
    continue;
    if (keys.size() == 0) {
      printf("empty histogram\n");
      continue;
    }
    
    printf("incrementing %lu bins\n", keys.size());
    const uint16_t szMultiOpIncrement = rc_multiOpSizeOf(MULTI_OP_INCREMENT);
    unsigned char *mIncrementObjects = reinterpret_cast<unsigned char *>
        (malloc(keys.size() * szMultiOpIncrement));
    void **pmIncrementObjects = reinterpret_cast<void **>
        (malloc(keys.size() * sizeof(pmIncrementObjects[0])));
    for (unsigned i = 0; i < keys.size(); ++i) {
        pmIncrementObjects[i] = mIncrementObjects + (i * szMultiOpIncrement);
        rc_multiIncrementCreate(tblid, &keys[i], sizeof(keys[0]),
                                0, values[i], NULL, pmIncrementObjects[i]);
    }
    rc_multiIncrement(client, pmIncrementObjects, keys.size());
    for (unsigned i = 0; i < keys.size(); ++i) {
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
  }
  
  printf("finished reading (%u tables)\n", ntables);
  return 0;
  
 /*  printf("write %u objects\n", N);
  val = 0;
  for (unsigned i = 0; i < N; ++i)
    ramcloud.write(tblid, &i, sizeof(i), &val, sizeof(val), NULL, NULL);
  printf("OK\n");

  printf("increment\n");
  val = 1;
  for (unsigned i = 0; i < N; ++i) 
    ramcloud.increment(tblid, &i, sizeof(i), val, NULL, NULL);
  printf("OK\n");*/

/*  printf("increment multi\n");
  MultiIncrementObject *multis[M];
  unsigned keys[M];
  Prng prng;
  prng.InitSeed(42);
  unsigned total = 0;
  double summand = 1.0;
 
  while (total < N) {
    for (unsigned i = 0; i < M; ++i) {
      keys[i] = prng.Next(B);
      multis[i] = new MultiIncrementObject(tblid, &keys[i], sizeof(keys[0]), 0, summand, NULL);
    }
    ramcloud.multiIncrement(multis, M);
    for (unsigned i = 0; i < M; ++i)
      delete multis[i];
    total += M;
    printf("processed %u bins\n", total); 
  }

  printf("OK\n");
  return 0;
*/
  /*printf("increment async\n");
  val = 1;
  IncrementFloatRpc *RPCs[M];
  unsigned keys[M];
  Prng prng;
  prng.InitSeed(42);
  unsigned total = 0;

  

  while (total < N) {
    for (unsigned i = 0; i < M; ++i) {
      keys[i] = prng.Next(100000);
      RPCs[i] = new IncrementFloatRpc(&ramcloud, tblid, &keys[i], sizeof(unsigned), val, NULL);
    }
    for (unsigned i = 0; i < M; ++i) {
 //     try {
        RPCs[i]->wait();
*//*      } catch (ClientException& e) {
        if (e.status == STATUS_OBJECT_DOESNT_EXIST) {
          RejectRules rrules;
          memset(&rrules, 0, sizeof(rrules));
          rrules.exists = 1;
          try {
            ramcloud.write(tblid, &keys[i], sizeof(keys[0]), &val, sizeof(val), &rrules, NULL);
          } catch (ClientException& e) {
            if (e.status == STATUS_OBJECT_EXISTS)
              ramcloud.increment(tblid, &keys[i], sizeof(unsigned), val, NULL);
          }
        }
      } */
//      delete RPCs[i];
//    }
//    total += M;
//  }
//  printf("OK\n");

  return 0;
}


