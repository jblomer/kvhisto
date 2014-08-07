#include <cstdlib>
#include <string>

#include "CRamCloud.h"

using namespace std;

typedef struct KvConnection KV_CONNECTION;
struct KvConnection {
  struct rc_client *client;
  string locator;
  string cluster_name;
  uint16_t num_nodes;
};

KV_CONNECTION *CppKvConnect(const char *locator, const uint16_t num_nodes) {
  KV_CONNECTION *conn = new KV_CONNECTION();
  conn->locator = locator;
  conn->cluster_name = "main";
  conn->num_nodes = num_nodes;
  Status status = rc_connect(locator, "main", &conn->client);
  if (status != STATUS_OK) {
    delete conn;
    return NULL;
  }
  return conn;
}


static bool 
GetTableId(KV_CONNECTION *conn, const char *tblname, uint64_t *tblid) {
  Status status;
  status = rc_getTableId(conn->client, tblname, tblid);
  if (status == STATUS_TABLE_DOESNT_EXIST) {
    status = rc_createTable(conn->client, tblname, conn->num_nodes);
    if (status != STATUS_OK)
      return false;
    status = rc_getTableId(conn->client, tblname, tblid);
  }
  if (status != STATUS_OK)
    return false;
  return true;
}


bool CppKvIncr(KV_CONNECTION *conn, const char *hist_name, uint32_t num_adds,
               uint64_t *bins, int64_t *values_int, double *values_float)
{
  uint16_t szMultiOpIncrement = rc_multiOpSizeOf(MULTI_OP_INCREMENT);
  uint64_t tblid;
  bool retval = GetTableId(conn, hist_name, &tblid);
  if (!retval)
    return false;
  
  unsigned char *mIncrementObjects = reinterpret_cast<unsigned char *>
      (malloc(num_adds * szMultiOpIncrement));
  void **pmIncrementObjects = reinterpret_cast<void **>
      (malloc(num_adds * sizeof(pmIncrementObjects[0])));
  uint16_t keyLength = sizeof(bins[0]);
  for (unsigned i = 0; i < num_adds; ++i) {
      pmIncrementObjects[i] = mIncrementObjects + (i * szMultiOpIncrement);
      rc_multiIncrementCreate(tblid, &bins[i], keyLength,
                              values_int[i], values_float[i], NULL, 
                              pmIncrementObjects[i]);
  }
  rc_multiIncrement(conn->client, pmIncrementObjects, num_adds);
  retval = true;
  for (unsigned i = 0; i < num_adds; ++i) {
      Status thisStatus = 
          rc_multiOpStatus(pmIncrementObjects[i], MULTI_OP_INCREMENT);
      if (thisStatus != STATUS_OK)
        retval = false;
      rc_multiOpDestroy(pmIncrementObjects[i], MULTI_OP_INCREMENT);
  }
  free(pmIncrementObjects);
  free(mIncrementObjects);
  return retval;
}                    


void CppKvDisconnect(KV_CONNECTION *conn) {
  rc_disconnect(conn->client);
  delete conn;
}


extern "C" {

KV_CONNECTION *KvConnect(const char *locator, const uint16_t num_nodes) {
  return CppKvConnect(locator, num_nodes);
}

void KvDisconnect(KV_CONNECTION *conn) {
  CppKvDisconnect(conn);
}

bool KvIncr(KV_CONNECTION *conn, const char *hist_name, uint32_t num_adds,
            uint64_t *bins, int64_t *values_int, double *values_float)
{
  return CppKvIncr(conn, hist_name, num_adds, bins, values_int, values_float);
}                   

}
