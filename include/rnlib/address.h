#ifndef RN_ADDRESS_H
#define RN_ADDRESS_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct RnAddressIPv4 {
  uint8_t octets[4];
  uint16_t port;
};

struct RnAddressIPv6 {
  uint16_t groups[8];
  uint16_t port;
};

struct RnAddressIPv4 rnAddressCreateIPv4( // LLVM 19
    uint8_t octets[4],
    uint16_t port
);

struct RnAddressIPv4 rnAddressCreateIPv4Args(
    // clang-format off
    uint8_t a, uint8_t b, uint8_t c, uint8_t d,
    // clang-format on
    uint16_t port
);

struct RnAddressIPv6 rnAddressCreateIPv6( // LLVM 19
    uint16_t groups[8],
    uint16_t port
);

struct RnAddressIPv6 rnAddressCreateIPv6Args(
    // clang-format off
    uint16_t a, uint16_t b, uint16_t c, uint16_t d,
    uint16_t e, uint16_t f, uint16_t g, uint16_t h,
    // clang-format on
    uint16_t port
);

#ifdef __cplusplus
} // extern C
#endif

#endif // RN_ADDRESS_H
