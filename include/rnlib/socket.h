#ifndef RN_SOCKET_H
#define RN_SOCKET_H

#include "address.h"
#include "result.h"

#ifdef __cplusplus
extern "C" {
#endif

struct RnSocketIPv4 {
  uint8_t data[4];
} __attribute__((aligned(4)));

struct RnSocketIPv6 {
  uint8_t data[4];
} __attribute__((aligned(4)));

rnresult_t rnSocketsSetup();

rnresult_t rnSocketsCleanup();

rnresult_t rnSocketOpenIPv4(
    _Out_ struct RnSocketIPv4 *socket,
    const struct RnAddressIPv4 *bind_to
);

rnresult_t rnSocketOpenIPv6(
    _Out_ struct RnSocketIPv6 *socket,
    const struct RnAddressIPv6 *bind_to
);

rnresult_t rnSocketCloseIPv4( // LLVM 19
    const struct RnSocketIPv4 *socket
);

rnresult_t rnSocketCloseIPv6( // LLVM 19
    const struct RnSocketIPv6 *socket
);

#ifdef __cplusplus
} // extern C
#endif

#endif // RN_SOCKET_H
