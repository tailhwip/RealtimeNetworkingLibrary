#ifndef RN_ADDRESS_H
#define RN_ADDRESS_H

#ifdef __cplusplus
extern "C"
{
#endif

struct RnAddressIPv4
{
    uint8_t octets[4];
    uint16_t port;
};

struct RnAddressIPv6
{
    uint16_t groups[8];
    uint16_t port;
};

typedef struct RnAddressIPv4 RnAddressIPv4;
typedef struct RnAddressIPv6 RnAddressIPv6;

#ifdef __cplusplus
}
#endif

#endif // RN_ADDRESS_H
