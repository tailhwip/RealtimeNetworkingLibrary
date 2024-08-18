#ifndef RN_ADDRESS_H_INTERNAL
#define RN_ADDRESS_H_INTERNAL

#include "../include/rnlib/address.h"

struct RxAddressIPv4 {
  uint8_t octets[4];
  uint16_t port;
};

struct RxAddressIPv6 {
  uint16_t groups[8];
  uint16_t port;
};

bool rxAddressEqualsIPv4( // LLVM 19
    const struct RxAddressIPv4 *left,
    const struct RxAddressIPv4 *right
);

bool rxAddressEqualsIPv6( // LLVM 19
    const struct RxAddressIPv6 *left,
    const struct RxAddressIPv6 *right
);

#endif // RN_ADDRESS_H_INTERNAL
