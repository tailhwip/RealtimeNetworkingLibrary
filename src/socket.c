#include "socket.h"

#include "address.h"
#include "packet.h"

#include <string.h>
#include <winsock2.h>

#ifdef _WIN32
  #include <ws2tcpip.h>
typedef int socklen_t;
#else
  #include <errno.h>
  #include <fcntl.h>
  #include <netinet/in.h>
  #include <sys/socket.h>
#endif

rnresult_t rnSocketsSetup() {
#ifdef _WIN32
  WSADATA data;
  int result = WSAStartup(MAKEWORD(2, 2), &data);
  if (result == SOCKET_ERROR) return WSAGetLastError();
#endif

  return RN_RESULT_OK;
}

rnresult_t rnSocketsCleanup() {
#ifdef _WIN32
  int result = WSACleanup();
  if (result == SOCKET_ERROR) return WSAGetLastError();
#endif

  return RN_RESULT_OK;
}

static struct sockaddr_in addressToAddrIPv4( // LLVM 19
    const struct RnAddressIPv4 *rn_address
) {
  struct AddressIPv4 *address = (struct AddressIPv4 *)rn_address;
  struct sockaddr_in addr = { .sin_family = AF_INET };
  memcpy(&addr.sin_addr.s_addr, address->octets, sizeof(address->octets));
  addr.sin_port = address->port;

  return addr;
}

static struct sockaddr_in6 addressToAddrIPv6( // LLVM 19
    const struct RnAddressIPv6 *rn_address
) {
  struct AddressIPv6 *address = (struct AddressIPv6 *)rn_address;
  struct sockaddr_in6 addr = { .sin6_family = AF_INET6 };
  memcpy(&addr.sin6_addr.u.Word, address->groups, sizeof(address->groups));
  addr.sin6_port = address->port;

  return addr;
}

static rnresult_t socketOpen(
    socket_h socket,
    const struct sockaddr *addr,
    size_t addr_size
) {
#ifdef _WIN32
  int bind_result = bind(socket, addr, addr_size);
  if (bind_result == SOCKET_ERROR) return WSAGetLastError();
#else
  int bind_result = bind(handle, address, address_size);
  if (bind_result == -1) return errno;
#endif

#ifdef _WIN32
  DWORD non_blocking = 1;
  int nbio_result = ioctlsocket(socket, FIONBIO, &non_blocking);
  if (nbio_result == SOCKET_ERROR) return WSAGetLastError();
#else
  int non_blocking = 1;
  int nbio_result = fcntl(handle, F_SETFL, O_NONBLOCK, non_blocking);
  if (nbio_result == -1) return errno;
#endif

  return RN_RESULT_OK;
}

rnresult_t rnSocketOpenIPv4(
    _Out_ struct RnSocketIPv4 *socket_out,
    const struct RnAddressIPv4 *bind_to
) {
  socket_h *handle = (socket_h *)socket_out;
  *handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  struct sockaddr_in addr = addressToAddrIPv4(bind_to);
  return socketOpen(*handle, (struct sockaddr *)&addr, sizeof addr);
}

rnresult_t rnSocketOpenIPv6(
    _Out_ struct RnSocketIPv6 *socket_out,
    const struct RnAddressIPv6 *bind_to
) {
  socket_h *handle = (socket_h *)socket_out;
  *handle = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

  struct sockaddr_in6 addr = addressToAddrIPv6(bind_to);
  return socketOpen(*handle, (struct sockaddr *)&addr, sizeof addr);
}

static rnresult_t socketClose(socket_h handle) {
#ifdef _WIN32
  int result = closesocket(handle);
  if (result == SOCKET_ERROR) return WSAGetLastError();
#else
  int result = close(handle);
  if (result == -1) return errno;
#endif

  return RN_RESULT_OK;
}

rnresult_t rnSocketCloseIPv4( // LLVM 19
    const struct RnSocketIPv4 *socket
) {
  socket_h handle = *(socket_h *)socket;
  return socketClose(handle);
}

rnresult_t rnSocketCloseIPv6( // LLVM 19
    const struct RnSocketIPv6 *socket
) {
  socket_h handle = *(socket_h *)socket;
  return socketClose(handle);
}

static rnresult_t socketSendData(
    socket_h socket,
    const struct sockaddr *addr,
    size_t addr_size,
    const void *data,
    size_t data_size
) {
  int result = sendto(socket, data, data_size, 0, addr, addr_size);
  if (result != data_size) {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
  }

  return RN_RESULT_OK;
}

rnresult_t socketSendDataIPv4(
    const struct RnSocketIPv4 *socket,
    const struct RnAddressIPv4 *send_to,
    const void *data,
    size_t data_size
) {
  struct sockaddr_in ipv4 = addressToAddrIPv4(send_to);
  struct sockaddr *addr = (struct sockaddr *)&ipv4;

  socket_h *handle = (socket_h *)socket;
  return socketSendData(*handle, addr, sizeof ipv4, data, data_size);
}

rnresult_t socketSendDataIPv6(
    const struct RnSocketIPv6 *socket,
    const struct RnAddressIPv6 *send_to,
    const void *data,
    size_t data_size
) {
  struct sockaddr_in6 ipv6 = addressToAddrIPv6(send_to);
  struct sockaddr *addr = (struct sockaddr *)&ipv6;

  socket_h *handle = (socket_h *)socket;
  return socketSendData(*handle, addr, sizeof ipv6, data, data_size);
}

static struct RnAddressIPv4 addrToAddressIPv4( // LLVM 19
    const struct sockaddr_in *addr
) {
  struct AddressIPv4 address;
  memcpy(address.octets, &addr->sin_addr.s_addr, sizeof(address.octets));
  address.port = addr->sin_port;

  return *(struct RnAddressIPv4 *)&address;
}

static struct RnAddressIPv6 addrToAddressIPv6( // LLVM 19
    const struct sockaddr_in6 *addr
) {
  struct AddressIPv6 address;
  memcpy(address.groups, &addr->sin6_addr.u.Word, sizeof(address.groups));
  address.port = addr->sin6_port;

  return *(struct RnAddressIPv6 *)&address;
}

rnresult_t socketReceiveDataIPv4(
    const struct RnSocketIPv4 *socket,
    _Out_ struct RnAddressIPv4 *receive_from,
    _Out_ void *data,
    _Out_ size_t *data_size
) {
  struct sockaddr_in ipv4;
  struct sockaddr *addr = (struct sockaddr *)&ipv4;
  socklen_t addr_size;

  socket_h *handle = (socket_h *)socket;
  int result = recvfrom(*handle, data, RN_PACKET_MAX_SIZE, 0, addr, &addr_size);
#ifdef _WIN32
  if (result == SOCKET_ERROR) return WSAGetLastError();
#else
  if (result == -1) return errno;
#endif

  *receive_from = addrToAddressIPv4(&ipv4);
  *data_size = result;
  return RN_RESULT_OK;
}

rnresult_t socketReceiveDataIPv6(
    const struct RnSocketIPv6 *socket,
    _Out_ struct RnAddressIPv6 *receive_from,
    _Out_ void *data,
    _Out_ size_t *data_size
) {
  struct sockaddr_in6 ipv6;
  struct sockaddr *addr = (struct sockaddr *)&ipv6;
  socklen_t addr_size;

  socket_h *handle = (socket_h *)socket;
  int result = recvfrom(*handle, data, RN_PACKET_MAX_SIZE, 0, addr, &addr_size);
#ifdef _WIN32
  if (result == SOCKET_ERROR) return WSAGetLastError();
#else
  if (result == -1) return errno;
#endif

  *receive_from = addrToAddressIPv6(&ipv6);
  *data_size = result;
  return RN_RESULT_OK;
}
