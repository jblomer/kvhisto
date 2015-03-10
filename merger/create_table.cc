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
    printf("Usage: %s <table name> <num_nodes> [locator]\n",
           progname);
}

int main(int argc, char **argv) {
  if (argc < 4) {
    Usage(argv[0]);
    return 1;
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
  
  const char *table_name = argv[1];
  const uint32_t num_nodes = atoi(argv[2]);
  rc_createTable(client, table_name, num_nodes);
  status = rc_getTableId(client, table_name, &global_tblid);
  if (status != STATUS_OK) {
    printf("failed getting global table id\n");
    abort();
  }
  
  printf("table %s is there\n", table_name);
  return 0;
}
