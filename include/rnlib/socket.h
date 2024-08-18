#ifndef RN_SOCKET_H
#define RN_SOCKET_H

#include "address.h"

#ifdef __cplusplus
extern "C" {
#endif

struct RnSocketIPv4 {
  uint8_t data[4];
} __attribute__((aligned(4)));

struct RnSocketIPv6 {
  uint8_t data[4];
} __attribute__((aligned(4)));

struct RnSocketsSetupResult rnSocketsSetup();

struct RnSocketsSetupResult {
  enum {
    RN_SOCKETS_SETUP_OK,
    RN_SOCKETS_SETUP_ERROR,
  } error;

  int error_code;
};

struct RnSocketsCleanupResult rnSocketsCleanup();

struct RnSocketsCleanupResult {
  enum {
    RN_SOCKETS_CLEANUP_OK,
    RN_SOCKETS_CLEANUP_ERROR,
  } error;

  int error_code;
};

struct RnSocketOpenResult rnSocketOpenIPv4(
    _Out_ struct RnSocketIPv4 *socket_out,
    const struct RnAddressIPv4 *bind_to_address
);

struct RnSocketOpenResult rnSocketOpenIPv6(
    _Out_ struct RnSocketIPv6 *socket_out,
    const struct RnAddressIPv6 *bind_to_address
);

struct RnSocketOpenResult {
  enum {
    RN_SOCKET_OPEN_OK,
    RN_SOCKET_OPEN_ERROR_BIND,
    RN_SOCKET_OPEN_ERROR_NBIO
  } error;

  int error_code;
};

struct RnSocketCloseResult rnSocketCloseIPv4( // LLVM 19
    const struct RnSocketIPv4 *socket
);

struct RnSocketCloseResult rnSocketCloseIPv6( // LLVM 19
    const struct RnSocketIPv6 *socket
);

struct RnSocketCloseResult {
  enum {
    RN_SOCKET_CLOSE_OK,
    RN_SOCKET_CLOSE_ERROR,
  } error;

  int error_code;
};

#ifdef __cplusplus
} // extern C
#endif

#endif // RN_SOCKET_H
