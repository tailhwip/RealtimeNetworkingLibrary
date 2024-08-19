#ifndef RN_SOCKET_H_INTERAL
#define RN_SOCKET_H_INTERAL

#include "../include/rnlib/socket.h"

typedef int socket_h;

rnresult_t socketSendDataIPv4(
    const struct RnSocketIPv4 *socket,
    const struct RnAddressIPv4 *send_to,
    const void *data,
    size_t data_size
);

rnresult_t socketSendDataIPv6(
    const struct RnSocketIPv6 *socket,
    const struct RnAddressIPv6 *send_to,
    const void *data,
    size_t data_size
);

rnresult_t socketReceiveDataIPv4(
    const struct RnSocketIPv4 *socket,
    _Out_ struct RnAddressIPv4 *receive_from,
    _Out_ void *data,
    _Out_ size_t *data_size
);

rnresult_t socketReceiveDataIPv6(
    const struct RnSocketIPv6 *socket,
    _Out_ struct RnAddressIPv6 *receive_from,
    _Out_ void *data,
    _Out_ size_t *data_size
);

#endif // RN_SOCKET_H_INTERAL
