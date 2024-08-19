#include "address.h"

#include <string.h>

// clang-format off
#ifdef _WIN32
  #include <ws2tcpip.h>
#else
  #include <netinet/in.h>
  #include <sys/socket.h>
#endif
// clang-format on

struct RnAddressIPv4 rnAddressCreateIPv4( // LLVM 19
    uint8_t octets[4],
    uint16_t port
) {
  struct AddressIPv4 address;
  *(uint16_t *)&address.octets[0] = htonl(*(uint16_t *)&octets[2]);
  *(uint16_t *)&address.octets[2] = htons(*(uint16_t *)&octets[0]);
  address.port = htons(port);

  return *(struct RnAddressIPv4 *)&address;
}

struct RnAddressIPv4 rnAddressCreateIPv4Args(
    // clang-format off
    uint8_t a, uint8_t b, uint8_t c, uint8_t d,
    // clang-format on
    uint16_t port
) {
  uint8_t octets[4] = { a, b, c, d };
  return rnAddressCreateIPv4(octets, port);
}

struct RnAddressIPv6 rnAddressCreateIPv6( // LLVM 19
    uint16_t groups[8],
    uint16_t port
) {
  struct AddressIPv6 address;
  for (int i = 0; i < 8; i += 1) {
    address.groups[i] = htons(groups[7 - i]);
  }
  address.port = htons(port);

  return *(struct RnAddressIPv6 *)&address;
}

struct RnAddressIPv6 rnAddressCreateIPv6Args(
    // clang-format off
    uint16_t a, uint16_t b, uint16_t c, uint16_t d,
    uint16_t e, uint16_t f, uint16_t g, uint16_t h,
    // clang-format on
    uint16_t port
) {
  uint16_t groups[8] = { a, b, c, d, e, f, g, h };
  return rnAddressCreateIPv6(groups, port);
}

bool addressEqualsIPv4(
    const struct AddressIPv4 *left,
    const struct AddressIPv4 *right
) {
  return memcmp(left, right, sizeof(struct RnAddressIPv4)) == 0;
}

bool addressEqualsIPv6(
    const struct AddressIPv6 *left,
    const struct AddressIPv6 *right
) {
  return memcmp(left, right, sizeof(struct RnAddressIPv6)) == 0;
}
