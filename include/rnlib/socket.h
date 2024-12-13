#ifndef RN_SOCKET_H
#define RN_SOCKET_H

#include "address.h"
#include "util.h"

#ifdef __cplusplus
extern "C"
{
#endif

int rnSocketsInitialize();
int rnSocketsCleanup();

#define RN_SOCKET_DECL(IP)                                                     \
    typedef struct RnSocket##IP RnSocket##IP;                                  \
                                                                               \
    int rnSocketOpen(const RnAddress##IP *host, OUT RnSocket##IP **handle);    \
                                                                               \
    int rnSocketClose(IN RnSocket##IP *handle);                                \
                                                                               \
    int rnSocketSendData(                                                      \
          const RnSocket##IP *socket, const RnAddress##IP *address,            \
          const uint8_t *data, size_t data_size);                              \
                                                                               \
    int rnSocketReceiveData(                                                   \
          const RnSocket##IP *socket, OUT RnAddress##IP *address,              \
          OUT uint8_t *data, OUT size_t *data_size);

RN_SOCKET_DECL(IPv4)
RN_SOCKET_DECL(IPv6)

#undef RN_SOCKET_DECL

#ifdef __cplusplus
}
#endif

#endif // RN_SOCKET_H
