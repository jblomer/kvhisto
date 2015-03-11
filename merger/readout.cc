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

struct rc_client *client;
uint64_t global_tblid;

void Usage(const char *progname) {
    printf("Usage: %s <table name> [locator]\n", progname);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    Usage(argv[0]);
    return 1;
  }
  
  Status status;
  const char *locator = "infrc:host=192.168.1.119,port=11100";
  if (argc >= 3)
    locator = argv[2];
  status = rc_connect(locator, "main", &client);
  if (status != STATUS_OK) {
    printf("failed connecting to RAMCloud\n");
    abort();
  }
  
  const char *table_name = argv[1];
  status = rc_getTableId(client, table_name, &global_tblid);
  if (status != STATUS_OK) {
    printf("failed getting global table id\n");
    abort();
  }
  

  void *enumState;
  rc_enumerateTablePrepare(client, global_tblid, 0, &enumState);
  uint32_t keyLen, dataLen;
  const void *key, *data;
  unsigned nObjects = 0;
  while (true) {
    keyLen = dataLen = 0;
    key = data = NULL;
    status = rc_enumerateTableNext(client, enumState,
            &keyLen, &key, &dataLen, &data);
    if (key == NULL)
      break;
    nObjects++;
    if (nObjects % 10000 == 0)
      printf("received another 10000 objects\n"); 
    if (nObjects % 1000000 == 0)
      printf("BREAKPOINT %u OBJECTS\n", nObjects);
  }
  rc_enumerateTableFinalize(enumState);  

  printf("Received %u objects\n", nObjects);

  return 0;
}
