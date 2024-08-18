#include "socket.h"

#include "address.h"

#include <string.h>

// clang-format off
#ifdef _WIN32
  #include <ws2tcpip.h>

  typedef int socklen_t;
#else
  #include <errno.h>
  #include <fcntl.h>
  #include <netinet/in.h>
  #include <sys/socket.h>
#endif
// clang-format on

#define RN_PACKET_MAX_SIZE 508

struct RnSocketsSetupResult rnSocketsSetup() {
#ifdef _WIN32
  WSADATA data;
  int error = WSAStartup(MAKEWORD(2, 2), &data);
  if (error) {
    return (struct RnSocketsSetupResult) {
      .error = RN_SOCKETS_SETUP_ERROR,
      .error_code = error,
    };
  }
#endif

  return (struct RnSocketsSetupResult) {
    .error = RN_SOCKETS_SETUP_OK,
  };
}

struct RnSocketsCleanupResult rnSocketsCleanup() {
#ifdef _WIN32
  int error = WSACleanup();
  if (error) {
    return (struct RnSocketsCleanupResult) {
      .error = RN_SOCKETS_CLEANUP_ERROR,
      .error_code = error,
    };
  }
#endif

  return (struct RnSocketsCleanupResult) {
    .error = RN_SOCKETS_CLEANUP_OK,
  };
}

static struct sockaddr_in rxAddressToAddrIPv4( // LLVM 19
    const struct RnAddressIPv4 *rn_address
) {
  struct RxAddressIPv4 *address = (struct RxAddressIPv4 *)rn_address;
  struct sockaddr_in addr = { .sin_family = AF_INET };
  memcpy(&addr.sin_addr.s_addr, address->octets, sizeof(address->octets));
  addr.sin_port = address->port;

  return addr;
}

static struct sockaddr_in6 rxAddressToAddrIPv6( // LLVM 19
    const struct RnAddressIPv6 *rn_address
) {
  struct RxAddressIPv6 *address = (struct RxAddressIPv6 *)rn_address;
  struct sockaddr_in6 addr = { .sin6_family = AF_INET6 };
  memcpy(&addr.sin6_addr.u.Word, address->groups, sizeof(address->groups));
  addr.sin6_port = address->port;

  return addr;
}

static struct RnSocketOpenResult rnSocketOpen(
    socket_h socket,
    const struct sockaddr *addr,
    size_t addr_size
) {
#ifdef _WIN32
  int bind_error = bind(socket, addr, addr_size);
#else
  errno = 0;
  bind(handle, address, address_size);
  int bind_error = errno;
#endif
  if (bind_error) {
    return (struct RnSocketOpenResult) {
      .error = RN_SOCKET_OPEN_ERROR_BIND,
      .error_code = bind_error,
    };
  }

#ifdef _WIN32
  DWORD non_blocking = 1;
  int nbio_error = ioctlsocket(socket, FIONBIO, &non_blocking);
#else
  fcntl(handle, F_SETFL, O_NONBLOCK, 1);
  int nbio_error = errno;
#endif
  if (nbio_error) {
    return (struct RnSocketOpenResult) {
      .error = RN_SOCKET_OPEN_ERROR_NBIO,
      .error_code = nbio_error,
    };
  }

  return (struct RnSocketOpenResult) {
    .error = RN_SOCKET_OPEN_OK,
  };
}

struct RnSocketOpenResult rnSocketOpenIPv4(
    _Out_ struct RnSocketIPv4 *socket_out,
    const struct RnAddressIPv4 *bind_to_address
) {
  socket_h *handle = (socket_h *)socket_out;
  *handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  struct sockaddr_in addr = rxAddressToAddrIPv4(bind_to_address);
  return rnSocketOpen(*handle, (struct sockaddr *)&addr, sizeof addr);
}

struct RnSocketOpenResult rnSocketOpenIPv6(
    _Out_ struct RnSocketIPv6 *socket_out,
    const struct RnAddressIPv6 *bind_to_address
) {
  socket_h *handle = (socket_h *)socket_out;
  *handle = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

  struct sockaddr_in6 addr = rxAddressToAddrIPv6(bind_to_address);
  return rnSocketOpen(*handle, (struct sockaddr *)&addr, sizeof addr);
}

static struct RnSocketCloseResult rnSocketClose(socket_h handle) {
#ifdef _WIN32
  int error = closesocket(handle);
#else
  errno = 0;
  close(handle);
  int error = errno;
#endif
  if (error) {
    return (struct RnSocketCloseResult) {
      .error = RN_SOCKET_CLOSE_ERROR,
      .error_code = error,
    };
  }

  return (struct RnSocketCloseResult) {
    .error = RN_SOCKET_CLOSE_OK,
  };
}

struct RnSocketCloseResult rnSocketCloseIPv4( // LLVM 19
    const struct RnSocketIPv4 *socket
) {
  socket_h handle = *(socket_h *)socket;
  return rnSocketClose(handle);
}

struct RnSocketCloseResult rnSocketCloseIPv6( // LLVM 19
    const struct RnSocketIPv6 *socket
) {
  socket_h handle = *(socket_h *)socket;
  return rnSocketClose(handle);
}

static struct RxSocketSendDataResult rxSocketSendData(
    socket_h socket,
    const struct sockaddr *addr,
    size_t addr_size,
    const void *data,
    size_t data_size
) {
  int bytes = sendto(socket, data, data_size, 0, addr, addr_size);
  if (bytes != data_size) {
    return (struct RxSocketSendDataResult) {
      .error = RX_SOCKET_SEND_DATA_ERROR,
      .error_code = 0, // TODO
    };
  }

  return (struct RxSocketSendDataResult) {
    .error = RX_SOCKET_SEND_DATA_OK,
  };
}

struct RxSocketSendDataResult rxSocketSendDataIPv4(
    const struct RnSocketIPv4 *socket,
    const struct RnAddressIPv4 *send_to_address,
    const void *data,
    size_t data_size
) {
  struct sockaddr_in ipv4 = rxAddressToAddrIPv4(send_to_address);
  struct sockaddr *addr = (struct sockaddr *)&ipv4;

  socket_h *handle = (socket_h *)socket;
  return rxSocketSendData(*handle, addr, sizeof ipv4, data, data_size);
}

struct RxSocketSendDataResult rxSocketSendDataIPv6(
    const struct RnSocketIPv6 *socket,
    const struct RnAddressIPv6 *send_to_address,
    const void *data,
    size_t data_size
) {
  struct sockaddr_in6 ipv6 = rxAddressToAddrIPv6(send_to_address);
  struct sockaddr *addr = (struct sockaddr *)&ipv6;

  socket_h *handle = (socket_h *)socket;
  return rxSocketSendData(*handle, addr, sizeof ipv6, data, data_size);
}

static struct RnAddressIPv4 rxAddrToAddressIPv4( // LLVM 19
    const struct sockaddr_in *addr
) {
  struct RxAddressIPv4 address;
  memcpy(address.octets, &addr->sin_addr.s_addr, sizeof(address.octets));
  address.port = addr->sin_port;

  return *(struct RnAddressIPv4 *)&address;
}

static struct RnAddressIPv6 rxAddrToAddressIPv6( // LLVM 19
    const struct sockaddr_in6 *addr
) {
  struct RxAddressIPv6 address;
  memcpy(address.groups, &addr->sin6_addr.u.Word, sizeof(address.groups));
  address.port = addr->sin6_port;

  return *(struct RnAddressIPv6 *)&address;
}

struct RxSocketReceiveDataResult rxSocketReceiveDataIPv4(
    const struct RnSocketIPv4 *socket,
    _Out_ struct RnAddressIPv4 *receive_from_address,
    _Out_ void *data,
    _Out_ size_t *data_size
) {
  struct sockaddr_in ipv4;
  struct sockaddr *addr = (struct sockaddr *)&ipv4;
  socklen_t addr_size;

  socket_h *handle = (socket_h *)socket;
  int bytes = recvfrom(*handle, data, RN_PACKET_MAX_SIZE, 0, addr, &addr_size);

  // TODO error
  // TODO set data_size

  *receive_from_address = rxAddrToAddressIPv4(&ipv4);
  return (struct RxSocketReceiveDataResult) {
    .error = RX_SOCKET_RECEIVE_DATA_OK,
  };
}

struct RxSocketReceiveDataResult rxSocketReceiveDataIPv6(
    const struct RnSocketIPv6 *socket,
    _Out_ struct RnAddressIPv6 *receive_from_address,
    _Out_ void *data,
    _Out_ size_t *data_size
) {
  struct sockaddr_in6 ipv6;
  struct sockaddr *addr = (struct sockaddr *)&ipv6;
  socklen_t addr_size;

  socket_h *handle = (socket_h *)socket;
  int bytes = recvfrom(*handle, data, RN_PACKET_MAX_SIZE, 0, addr, &addr_size);

  // TODO error
  // TODO set data_size

  *receive_from_address = rxAddrToAddressIPv6(&ipv6);
  return (struct RxSocketReceiveDataResult) {
    .error = RX_SOCKET_RECEIVE_DATA_OK,
  };
}
