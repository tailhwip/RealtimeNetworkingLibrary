#ifndef RN_SOCKET_H_INTERAL
#define RN_SOCKET_H_INTERAL

#include "../include/rnlib/socket.h"

typedef int socket_h;

struct RxSocketSendDataResult rxSocketSendDataIPv4(
    const struct RnSocketIPv4 *socket,
    const struct RnAddressIPv4 *send_to_address,
    const void *data,
    size_t data_size
);

struct RxSocketSendDataResult rxSocketSendDataIPv6(
    const struct RnSocketIPv6 *socket,
    const struct RnAddressIPv6 *send_to_address,
    const void *data,
    size_t data_size
);

struct RxSocketSendDataResult {
  enum {
    RX_SOCKET_SEND_DATA_OK,
    RX_SOCKET_SEND_DATA_ERROR,
  } error;

  int error_code;
};

struct RxSocketReceiveDataResult rxSocketReceiveDataIPv4(
    const struct RnSocketIPv4 *socket,
    _Out_ struct RnAddressIPv4 *receive_from_address,
    _Out_ void *data,
    _Out_ size_t *data_size
);

struct RxSocketReceiveDataResult rxSocketReceiveDataIPv6(
    const struct RnSocketIPv6 *socket,
    _Out_ struct RnAddressIPv6 *receive_from_address,
    _Out_ void *data,
    _Out_ size_t *data_size
);

struct RxSocketReceiveDataResult {
  enum {
    RX_SOCKET_RECEIVE_DATA_OK,
    RX_SOCKET_RECEIVE_DATA_ERROR,
  } error;

  int error_code;
};

#endif // RN_SOCKET_H_INTERAL
