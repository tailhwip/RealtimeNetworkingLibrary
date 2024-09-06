#pragma once

#include <array>
#include <cstdint>
#include <optional>

#define RN_PACKET_SIZE_MAX 508
#define RN_PACKET_TYPE_PROTOCOL UINT8_MAX

#ifndef RN_PROTOCOL_ID
    #define RN_PROTOCOL_ID 'r' + 'n'
#endif

namespace rn
{

struct PacketMeta
{
    _Inout_ uint32_t byte_size   : 9;
    _Out_ uint32_t connection_id : 23;
};

struct PacketHeader
{
    uint8_t protocol;
    uint8_t type;
    uint16_t sequence;

    inline operator uint8_t *() { return *this; }
    inline operator const uint8_t *() const { return *this; }
};

template <size_t qwords>
struct PacketBody : std::array<uint64_t, qwords>
{
    inline operator uint8_t *() { return *this; }
    inline operator const uint8_t *() const { return *this; }
};

struct PacketAuth : std::array<uint8_t, 16>
{
    inline operator uint8_t *() { return *this; }
    inline operator const uint8_t *() const { return *this; }
};

struct PacketChecksum
{
    uint32_t salt_upper;
    uint32_t salt_lower : 16;
    uint32_t crc16      : 16;
};

class PacketBuffer
{
private:
    uint64_t *body;
    uint_fast8_t body_size;
    uint_fast8_t body_cursor;

    uint64_t scratch;
    uint_fast8_t scratch_cursor;

protected:
    PacketBuffer(uint64_t *body, uint_fast8_t body_size);

    template <typename T>
    inline void serialize(T data, uint_fast8_t bit_count);

public:
    void serialize_bool(bool);
    void serialize_uint8(uint8_t);
    void serialize_uint16(uint16_t);
    void serialize_uint32(uint32_t);
    void serialize_uint64(uint64_t);
};

class SecurePacketBuffer : PacketBuffer
{
private:
    static constexpr uint_fast8_t qwords = 61;

public:
    PacketMeta meta;
    PacketAuth auth;
    PacketHeader header;
    PacketBody<qwords> body;

    SecurePacketBuffer(uint8_t packet_type);
};

class InsecurePacketBuffer : PacketBuffer
{
private:
    static constexpr uint_fast8_t qwords = 62;

public:
    PacketMeta meta;
    PacketChecksum checksum;
    PacketHeader header;
    PacketBody<qwords> body;

    InsecurePacketBuffer(uint8_t packet_type);
};

struct PacketSequence
{
    union
    {
        struct
        {
            uint16_t generation;
            uint16_t number;
        };

        /**
         * Secure packets require a unique nonce for each packet created during
         * a single session. These need not be private, thus this union combines
         * the sequence number and generation to create an incrementing sequence
         * sufficiently large for most, if not all, use cases.
         *
         * Technically, this limits a secure session to 2^32 - 1 packets each
         * way before their cryptographic integrity breaks.
         */
        uint32_t nonce;
    };

    /**
     * Increments this sequence by 1 and increments the generation if this
     * number has reached its maximum value.
     */
    uint16_t operator++();

    /**
     * Attempts to merge the given number into this sequence, returning
     * std::nullopt if it is too out of sync with this number.
     */
    std::optional<PacketSequence> operator<<(uint16_t number);
};

} // namespace rn
