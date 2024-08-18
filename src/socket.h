#ifndef RN_SOCKET_H_INTERAL
#define RN_SOCKET_H_INTERAL

#include "../include/rnlib/socket.h"

struct RnSocketSendDataResult rnSocketSendDataIPv4(
    const struct RnSocketIPv4 *socket,
    const struct RnAddressIPv4 *send_to,
    const void *data,
    size_t data_size
);

struct RnSocketSendDataResult rnSocketSendDataIPv6(
    const struct RnSocketIPv6 *socket,
    const struct RnAddressIPv6 *send_to,
    const void *data,
    size_t data_size
);

struct RnSocketSendDataResult {
  enum {
    RN_SOCKET_SEND_DATA_OK,
    RN_SOCKET_SEND_DATA_ERROR,
  } error;

  int error_code;
};

struct RnSocketReceiveDataResult rnSocketReceiveDataIPv4(
    const struct RnSocketIPv4 *socket,
    _Out_ struct RnAddressIPv4 *receive_from,
    _Out_ void *data,
    size_t data_size
);

struct RnSocketReceiveDataResult rnSocketReceiveDataIPv6(
    const struct RnSocketIPv6 *socket,
    _Out_ struct RnAddressIPv6 *receive_from,
    _Out_ void *data,
    size_t data_size
);

struct RnSocketReceiveDataResult {
  enum {
    RN_SOCKET_RECEIVE_DATA_OK,
    RN_SOCKET_RECEIVE_DATA_ERROR,
  } error;

  int error_code;
};

#endif // RN_SOCKET_H_INTERAL
