#include "address.h"

// clang-format off
#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <netinet/in.h>
    #include <sys/socket.h>
#endif
// clang-format on

struct RnAddressIPv4 rnAddressCreateIPv4( // LLVM 19
    uint8_t octets[4],
    uint16_t port
) {
  struct RnAddressIPv4 address;
  *(uint32_t *)address.octets = htonl(*(uint32_t *)octets);
  address.port = htons(port);

  return address;
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
  struct RnAddressIPv6 address;
  *(uint64_t *)&address.groups[0] = htonll(*(uint64_t *)&groups[4]);
  *(uint64_t *)&address.groups[4] = htonll(*(uint64_t *)&groups[0]);
  address.port = htons(port);

  return address;
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

bool rnAddressEqualsIPv4(
    const struct RnAddressIPv4 *left,
    const struct RnAddressIPv4 *right
) {
  uint32_t left_ipv4 = *(uint32_t *)left->octets;
  uint32_t right_ipv4 = *(uint32_t *)right->octets;
  return (left_ipv4 == right_ipv4) && (left->port == right->port);
}

bool rnAddressEqualsIPv6(
    const struct RnAddressIPv6 *left,
    const struct RnAddressIPv6 *right
) {
  uint64_t left_ipv6_head = *(uint64_t *)&left->groups[0];
  uint64_t left_ipv6_tail = *(uint64_t *)&left->groups[4];

  uint64_t right_ipv6_head = *(uint64_t *)&right->groups[0];
  uint64_t right_ipv6_tail = *(uint64_t *)&right->groups[4];

  return (left_ipv6_head == right_ipv6_head) &&
         (left_ipv6_tail == right_ipv6_tail) && (left->port == right->port);
}
