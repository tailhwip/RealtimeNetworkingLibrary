#ifndef RN_ADDRESS_H_INTERNAL
#define RN_ADDRESS_H_INTERNAL

#include "../include/rnlib/address.h"

struct AddressIPv4 {
  uint8_t octets[4];
  uint16_t port;
};

struct AddressIPv6 {
  uint16_t groups[8];
  uint16_t port;
};

bool addressEqualsIPv4( // LLVM 19
    const struct AddressIPv4 *left,
    const struct AddressIPv4 *right
);

bool addressEqualsIPv6( // LLVM 19
    const struct AddressIPv6 *left,
    const struct AddressIPv6 *right
);

#endif // RN_ADDRESS_H_INTERNAL
