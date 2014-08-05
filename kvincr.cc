#include <cstdlib>
#include <string>

typedef struct KvConnection KV_CONNECTION;
struct KvConnection {
  char *locator;
  char *cluster_name;
};

KV_CONNECTION *KvConnect(const char *locator) {
  return NULL;
}

bool KvIncrInt(KV_CONNECTION *conn, const char *hist_name, uint32_t num_adds,
               uint64_t *bins, uint64_t *values[])
{
  return false;
}              
   
bool KvIncrFloat(KV_CONNECTION *conn, const char *hist_name, uint32_t num_adds,
                 uint64_t *bins, uint64_t *values[])
{
  return false;
}            

void KvDisconnect(KV_CONNECTION *conn) {
  
}
