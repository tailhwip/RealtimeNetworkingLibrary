#ifndef RN_PACKET_H
#define RN_PACKET_H

#include "cryptography.h"

#include <stdint.h>

#define RN_NET_MTU_MIN_IPV4 576
#define RN_NET_MTU_MIN_IPV6 1280

#define RN_UDP_HEADER_MIN_IPV4 28
#define RN_UDP_HEADER_MIN_IPV6 48

#define RN_NET_PAYLOAD_SAFE_IPV4 RN_NET_MTU_MIN_IPV4 - RN_UDP_HEADER_MIN_IPV4
#define RN_NET_PAYLOAD_SAFE_IPV6 RN_NET_MTU_MIN_IPV6 - RN_UDP_HEADER_MIN_IPV6

#ifndef RN_PACKET_BYTES_MAX
    #define RN_PACKET_BYTES_MAX RN_NET_PAYLOAD_SAFE_IPV6
#endif

#ifdef __cplusplus
extern "C"
{
#endif

struct RnPacketBufferMetaData
{
    uint32_t connection_id;
    uint16_t body_size;
};

struct RnPacketBufferCursor
{
    uint_fast16_t qword;
    uint_fast8_t bit;
};

struct RnPacketHeader
{
    uint16_t protocol;
    uint16_t sequence;
    uint16_t acknowledged;
    uint16_t type;
};

typedef struct RnPacketHeader RnPacketHeader;

struct RnPacketBufferSecure
{
    uint8_t[16] auth;
    PacketHeader head;
    uint64_t body[RN_PACKET_BYTES_MAX / 8 - 3];

} __attribute__((aligned(64)));

struct RnPacketBufferInsecure
{
    uint64_t salt;
    PacketHeader head;
    uint64_t body[RN_PACKET_BYTES_MAX / 8 - 2];

} __attribute__((aligned(64)));

typedef struct RnPacketBufferSecure RnPacketBufferSecure;
typedef struct RnPacketBufferInsecure RnPacketBufferInsecure;

RnPacketBufferSecure rnPacketBufferCreateSecure(uint16_t type);
RnPacketBufferInsecure rnPacketBufferCreateInsecure(uint16_t type);

int rnPacketBufferSerializeBool(RnPacketBufferSecure *, bool);
int rnPacketBufferSerializeBool(RnPacketBufferInsecure *, bool);

int rnPacketBufferSerializeUInt8(RnPacketBufferSecure *, uint8_t);
int rnPacketBufferSerializeUInt8(RnPacketBufferInsecure *, uint8_t);

int rnPacketBufferSerializeUInt16(RnPacketBufferSecure *, uint16_t);
int rnPacketBufferSerializeUInt16(RnPacketBufferInsecure *, uint16_t);

int rnPacketBufferSerializeUInt32(RnPacketBufferSecure *, uint32_t);
int rnPacketBufferSerializeUInt32(RnPacketBufferInsecure *, uint32_t);

int rnPacketBufferSerializeUInt64(RnPacketBufferSecure *, uint64_t);
int rnPacketBufferSerializeUInt64(RnPacketBufferInsecure *, uint64_t);

int rnPacketBufferSerializeKeyBuffer(RnPacketBufferSecure *, const RnKeyBuffer *);
int rnPacketBufferSerializeKeyBuffer(RnPacketBufferInsecure *, const RnKeyBuffer *);

void rnPacketBufferSerializePadding(RnPacketBufferSecure *);
void rnPacketBufferSerializePadding(RnPacketBufferInsecure *);

#ifdef __cplusplus
}
#endif

#endif // RN_PACKET_H
