#pragma once

#include "cryptography.h"

#include <array>
#include <cstdint>

namespace rn
{

#define NET_MTU_BYTES_MIN_IPV4 576
#define NET_MTU_BYTES_MIN_IPV6 1280

#define UDP_HEADER_BYTES_MIN_IPV4 28
#define UDP_HEADER_BYTES_MIN_IPV6 48

#define PAYLOAD_BYTES_SAFE_IPV4                                                \
    NET_MTU_BYTES_MIN_IPV4 - UDP_HEADER_BYTES_MIN_IPV4
#define PAYLOAD_BYTES_SAFE_IPV6                                                \
    NET_MTU_BYTES_MIN_IPV6 - UDP_HEADER_BYTES_MIN_IPV6

#ifndef PACKET_BYTES_MAX
#define PACKET_BYTES_MAX PAYLOAD_BYTES_SAFE_IPV6
#endif

#define PACKET_TYPE_HANDSHAKE UINT16_MAX

struct PacketMeta
{
    uint32_t connection_id;
    uint16_t body_size;
    uint16_t body_cursor;
};

struct PacketAuthentication : std::array<uint8_t, 16>
{
    inline operator uint8_t *() { return *this; }
    inline operator const uint8_t *() const { return *this; }
};

struct PacketVerification
{
    uint64_t salt;
};

struct PacketHeader
{
    uint16_t protocol;
    uint16_t sequence;
    uint16_t handshake;
    uint16_t type;

    inline operator uint8_t *() { return *this; }
    inline operator const uint8_t *() const { return *this; }
};

template <size_t qwords>
struct PacketBody : std::array<uint64_t, qwords>
{
    inline operator uint8_t *() { return *this; }
    inline operator const uint8_t *() const { return *this; }
};

template <typename TAG_T>
class PacketBuffer
{
private:
    template <typename T, size_t bits>
    inline void serialize(T data);

    template <typename T, size_t bits>
    inline T deserialize();

public:
    static constexpr size_t header_size = sizeof(TAG_T) + sizeof(PacketHeader);
    static constexpr size_t body_qwords = (PACKET_BYTES_MAX - header_size) / 8;

    PacketMeta meta;
    TAG_T tag;
    PacketHeader header;
    PacketBody<body_qwords> body;

    PacketBuffer(uint16_t packet_type);

    inline uint8_t *data() { return &tag; }
    inline const uint8_t *data() const { return &tag; }

    template <typename T>
    void SerializeT(T);

    void SerializeBool(bool);
    void SerializeUInt8(uint8_t); // TODO impl min/max values
    void SerializeUInt16(uint16_t);
    void SerializeUInt32(uint32_t);
    void SerializeUInt64(uint64_t);
    void SerializeKeyBuffer(const KeyBuffer &);
    void SerializePadding();

    template <typename T>
    T DeserializeT();

    bool DeserializeBool();
    uint8_t DeserializeUInt8();
    uint16_t DeserializeUInt16();
    uint32_t DeserializeUInt32();
    uint64_t DeserializeUInt64();
    KeyBuffer DeserializeKeyBuffer();
};

using SecurePacketBuffer = PacketBuffer<PacketAuthentication>;
using InsecurePacketBuffer = PacketBuffer<PacketVerification>;

} // namespace rn
