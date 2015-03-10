
#include <cstdio>
#include <inttypes.h>
#include <string>

#include <ramcloud/RamCloud.h>
#include <ramcloud/TableEnumerator.h>

const unsigned N = 10000000;
const unsigned B = 10000000;
const unsigned M = 10000;

using namespace RAMCloud;

int main(int argc, char **argv) {
  const char *locator = "infrc:host=192.168.1.119,port=11100";
  if (argc >= 3)
    locator = argv[2];
  RamCloud ramcloud(locator);
  
  uint64_t tblid = ramcloud.getTableId(argv[1]);
  unsigned dummy = unsigned(-1);
  std::string table_locator = ramcloud.testingGetServiceLocator(tblid, &dummy, sizeof(dummy));
  printf("table is on %s\n", table_locator.c_str());
 
  unsigned total = 0;
  TableEnumerator tabenum(ramcloud, tblid, false);
  while (tabenum.hasNext()) {
    uint32_t size;
    const void *object;
    tabenum.next(&size, &object);
    total++;
    if (total % 10000 == 0)
      printf("Enumerated %u objects\n", total);
  }
  
  printf("OK (%u)\n", total);
  return 0;

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


