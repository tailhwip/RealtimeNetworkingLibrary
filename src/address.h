#ifndef RN_ADDRESS_H_INTERNAL
#define RN_ADDRESS_H_INTERNAL

#include "../include/rnlib/address.h"

bool rnAddressEqualsIPv4( // LLVM 19
    const struct RnAddressIPv4 *left,
    const struct RnAddressIPv4 *right
);

bool rnAddressEqualsIPv6( // LLVM 19
    const struct RnAddressIPv6 *left,
    const struct RnAddressIPv6 *right
);

#endif // RN_ADDRESS_H_INTERNAL
