#include "socket.h"

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

typedef int socket_h;

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

static struct RnSocketOpenResult rnSocketOpen(
    socket_h socket,
    const struct sockaddr *address,
    size_t address_size
) {
#ifdef _WIN32
  int bind_error = bind(socket, address, address_size);
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
    _Out_ struct RnSocketIPv4 *out,
    const struct RnAddressIPv4 *bind_to
) {
  out->handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  struct sockaddr_in ipv4 = { .sin_family = AF_INET };
  ipv4.sin_addr.s_addr = *(uint32_t *)bind_to->octets;
  ipv4.sin_port = bind_to->port;

  return rnSocketOpen(out->handle, (struct sockaddr *)&ipv4, sizeof ipv4);
}

struct RnSocketOpenResult rnSocketOpenIPv6(
    _Out_ struct RnSocketIPv6 *out,
    const struct RnAddressIPv6 *bind_to
) {
  out->handle = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

  struct sockaddr_in6 ipv6 = { .sin6_family = AF_INET6 };
  ipv6.sin6_addr.u.Word[0] = *(uint64_t *)&bind_to->groups[0];
  ipv6.sin6_addr.u.Word[4] = *(uint64_t *)&bind_to->groups[4];
  ipv6.sin6_port = bind_to->port;

  return rnSocketOpen(out->handle, (struct sockaddr *)&ipv6, sizeof ipv6);
}

static struct RnSocketCloseResult rnSocketClose(int handle) {
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
  return rnSocketClose(socket->handle);
}

struct RnSocketCloseResult rnSocketCloseIPv6( // LLVM 19
    const struct RnSocketIPv6 *socket
) {
  return rnSocketClose(socket->handle);
}

static struct RnSocketSendDataResult rnSocketSendData(
    socket_h socket,
    const struct sockaddr *address,
    size_t address_size,
    const void *data,
    size_t data_size
) {
  int bytes = sendto(socket, data, data_size, 0, address, address_size);
  if (bytes != data_size) {
    return (struct RnSocketSendDataResult) {
      .error = RN_SOCKET_SEND_DATA_ERROR,
      .error_code = 0, // TODO
    };
  }

  return (struct RnSocketSendDataResult) {
    .error = RN_SOCKET_SEND_DATA_OK,
  };
}

struct RnSocketSendDataResult rnSocketSendDataIPv4(
    const struct RnSocketIPv4 *socket,
    const struct RnAddressIPv4 *send_to,
    const void *data,
    size_t data_size
) {
  struct sockaddr_in ipv4 = { .sin_family = AF_INET };
  ipv4.sin_addr.s_addr = *(uint32_t *)send_to->octets;
  ipv4.sin_port = send_to->port;

  struct sockaddr *addr = (struct sockaddr *)&ipv4;
  return rnSocketSendData(socket->handle, addr, sizeof ipv4, data, data_size);
}

struct RnSocketSendDataResult rnSocketSendDataIPv6(
    const struct RnSocketIPv6 *socket,
    const struct RnAddressIPv6 *send_to,
    const void *data,
    size_t data_size
) {
  struct sockaddr_in6 ipv6 = { .sin6_family = AF_INET6 };
  *(uint64_t *)&ipv6.sin6_addr.u.Word[0] = *(uint64_t *)&send_to->groups[0];
  *(uint64_t *)&ipv6.sin6_addr.u.Word[4] = *(uint64_t *)&send_to->groups[4];
  ipv6.sin6_port = send_to->port;

  struct sockaddr *addr = (struct sockaddr *)&ipv6;
  return rnSocketSendData(socket->handle, addr, sizeof ipv6, data, data_size);
}

struct RnSocketReceiveDataResult rnSocketReceiveDataIPv4(
    const struct RnSocketIPv4 *socket,
    _Out_ struct RnAddressIPv4 *recv_from,
    _Out_ void *data,
    size_t data_size
) {
  struct sockaddr_in ipv4;
  struct sockaddr *addr = (struct sockaddr *)&ipv4;
  socklen_t addr_size;
  int bytes = recvfrom(socket->handle, data, data_size, 0, addr, &addr_size);

  *(uint32_t *)recv_from->octets = ipv4.sin_addr.s_addr;
  recv_from->port = ipv4.sin_port;

  // TODO error

  return (struct RnSocketReceiveDataResult) {
    .error = RN_SOCKET_RECEIVE_DATA_OK,
  };
}

struct RnSocketReceiveDataResult rnSocketReceiveDataIPv6(
    const struct RnSocketIPv6 *socket,
    _Out_ struct RnAddressIPv6 *recv_from,
    _Out_ void *data,
    size_t data_size
) {
  struct sockaddr_in6 ipv6;
  struct sockaddr *addr = (struct sockaddr *)&ipv6;
  socklen_t addr_size;
  int bytes = recvfrom(socket->handle, data, data_size, 0, addr, &addr_size);

  *(uint64_t *)&recv_from->groups[0] = *(uint64_t *)&ipv6.sin6_addr.u.Word[0];
  *(uint64_t *)&recv_from->groups[4] = *(uint64_t *)&ipv6.sin6_addr.u.Word[4];
  recv_from->port = ipv6.sin6_port;

  // TODO error

  return (struct RnSocketReceiveDataResult) {
    .error = RN_SOCKET_RECEIVE_DATA_OK,
  };
}
