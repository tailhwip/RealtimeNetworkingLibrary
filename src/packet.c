#include "../include/rnlib/packet.h"

#include <assert.h>

RnPacketBufferSecure rnPacketBufferCreateSecure(uint16_t type)
{
    return RnPacketBufferSecure {
        .auth = { 0 },
        .head = { 0, 0, 0, type },
        .body = { 0 },
    };
};

RnPacketBufferInsecure rnPacketBufferCreateInsecure(uint16_t type)
{
    return RnPacketBufferInsecure {
        .salt = 0,
        .head = { 0, 0, 0, type },
        .body = { 0 },
    };
};

inline int rxPacketBufferWrite(
      uint64_t *body, size_t body_qwords, RnPacketBufferCursor *cursor, uint64_t data,
      size_t data_bits)
{
    int_fast8_t bits_available = 64 - cursor->bit;
    uint64_t overflow_mask     = -uint64_t(data_bits > bits_available);

    assert(cursor->word < (body_qwords - 1) || !overflow_mask);

    // store value in current qword at bit cursor
    body[cursor->qword] |= data << cursor->bit;

    // increment qword cursor on overflow
    cursor->qword += overflow_mask & 1;

    // store remainder in next qword on overflow
    body[cursor->qword] |= overflow_mask & (data >> bits_available);

    // increment bit cursor by bits serialized
    cursor->bit = cursor->bit + data_bits - overflow_mask & 64;
}

#define RN_PACKET_BUFFER_WRITE_INT_IMPL(BUFFER, NAME, TYPE, BITS)                                  \
    inline int rnPacketBufferWrite##NAME(                                                          \
          RnPacketBuffer##BUFFER *buffer, RnPacketBufferCursor *cursor, TYPE data)                 \
    {                                                                                              \
        return rxPacketBufferWrite(buffer->body, sizeof buffer->body / 8, cursor, data, BITS);     \
    }                                                                                              \
                                                                                                   \
    inline int rnPacketBufferWrite(                                                                \
          RnPacketBuffer##BUFFER *buffer, RnPacketBufferCursor *cursor, TYPE data)                 \
    {                                                                                              \
        return rnPacketBufferWrite##NAME(buffer, cursor, data);                                    \
    }

RN_PACKET_BUFFER_WRITE_INT_IMPL(Secure, Bool, bool, 1)
RN_PACKET_BUFFER_WRITE_INT_IMPL(Insecure, Bool, bool, 1)

RN_PACKET_BUFFER_WRITE_INT_IMPL(Secure, UInt8, uint8_t, 8)
RN_PACKET_BUFFER_WRITE_INT_IMPL(Insecure, UInt8, uint8_t, 8)

RN_PACKET_BUFFER_WRITE_INT_IMPL(Secure, UInt16, uint16_t, 16)
RN_PACKET_BUFFER_WRITE_INT_IMPL(Insecure, UInt16, uint16_t, 16)

RN_PACKET_BUFFER_WRITE_INT_IMPL(Secure, UInt32, uint32_t, 32)
RN_PACKET_BUFFER_WRITE_INT_IMPL(Insecure, UInt32, uint32_t, 32)

RN_PACKET_BUFFER_WRITE_INT_IMPL(Secure, UInt64, uint64_t, 64)
RN_PACKET_BUFFER_WRITE_INT_IMPL(Insecure, UInt64, uint64_t, 64)

#define RN_PACKET_BUFFER_WRITE_MISC_IMPL(BUFFER)                                                   \
    int rnPacketBufferWriteKey(                                                                    \
          RnPacketBuffer##BUFFER *buffer, RnPacketBufferCursor *cursor, const RnKeyBuffer *key)    \
    {                                                                                              \
        const uint64_t *data = key->data;                                                          \
        for (int i = 0; i < 4; i += 8)                                                             \
        {                                                                                          \
            rnPacketSerializeUInt64(buffer, cursor, *data[i]);                                     \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    void rnPacketBufferWritePadding(RnPacketBuffer##BUFFER *buffer, RnPacketBufferCursor *cursor)  \
    {                                                                                              \
        cursor->qword = sizeof buffer->body / 8;                                                   \
        cursor->bit   = 0;                                                                         \
    }

RN_PACKET_BUFFER_WRITE_MISC_IMPL(Secure)
RN_PACKET_BUFFER_WRITE_MISC_IMPL(Insecure)

int rnPacketBufferAuthenticate(RnPacketBuffer *buffer, RnPacketMetaData meta, )
{
}

int rnPacketBufferVerifyAuthentication()
{
}

int rnPacketBufferEncrypt()
{
}

int rnPacketBufferDecrypt()
{
}

int rnPacketBufferTagSalt()
{
}

int rnPacketBufferVerifySalt()
{
}
