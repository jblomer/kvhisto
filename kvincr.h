#ifndef KVINCR_H
#define KVINCR_H

#include <inttypes.h>

#include <string>

#ifdef __cplusplus
extern "C" {
#else
typedef bool int;
#endif

typedef void KV_CONNECTION;

KV_CONNECTION *KvConnect(const char *locator, const uint16_t num_nodes);
bool KvIncr(KV_CONNECTION *conn, const char *hist_name, uint32_t num_adds,
            uint64_t *bins, int64_t *values_int, double *values_float);
void KvDisconnect(KV_CONNECTION *conn);


#ifdef __cplusplus
}
#endif

#endif  // KVINCR_H
