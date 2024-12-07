#include "../include/rnlib/packet.h"

using namespace rn;

template <typename T>
PacketBuffer<T>::PacketBuffer(uint16_t packet_type)
    : meta {}, tag {}, header { .type = packet_type }, body {}
{
}

template <typename TAG_T>
template <typename T, size_t bits>
inline void PacketBuffer<TAG_T>::serialize(T x)
{
    uint_fast16_t qword_cursor = meta.body_cursor % 64;
    int_fast8_t bits_available = 64 - qword_cursor;
    uint64_t overflow_mask = -uint64_t(bits > bits_available);

    // store value in current qword at bit cursor
    body[meta.body_size] |= uint64_t(x) << qword_cursor;

    // increment body size on qword overflow
    meta.body_size += overflow_mask & 1;

    // store remainder in next qword on overflow
    body[meta.body_size] |= overflow_mask & (uint64_t(x) >> bits_available);

    // increment body cursor by bits serialized
    meta.body_cursor += bits;
}

template <typename T>
void PacketBuffer<T>::SerializeBool(bool b)
{
    serialize<bool, 1>(b);
}

template <typename T>
void PacketBuffer<T>::SerializeUInt8(uint8_t x)
{
    serialize<uint8_t, 8>(x);
}

template <typename T>
void PacketBuffer<T>::SerializeUInt16(uint16_t x)
{
    serialize<uint16_t, 16>(x);
}

template <typename T>
void PacketBuffer<T>::SerializeUInt32(uint32_t x)
{
    serialize<uint32_t, 32>(x);
}

template <typename T>
void PacketBuffer<T>::SerializeUInt64(uint64_t x)
{
    serialize<uint64_t, 64>(x);
}

template <typename T>
void PacketBuffer<T>::SerializeKeyBuffer(const KeyBuffer &buffer)
{
    for (auto &qword : buffer)
    {
        SerializeUInt64(qword);
    }
}

template <typename T>
void PacketBuffer<T>::SerializePadding()
{
    meta.body_size = body_qwords;
    meta.body_cursor = body_qwords * 64;
}

template <typename TAG_T>
template <typename T, size_t bits>
inline T PacketBuffer<TAG_T>::deserialize() // TODO
{
    uint_fast16_t qword_cursor = meta.body_cursor % 64;
    uint64_t overflow_mask = -uint64_t(bits > qword_cursor);

    // store value in current qword at bit cursor
    body[meta.body_size] |= uint64_t(x) << qword_cursor;

    // set next qword to remainder on overflow
    body[meta.body_size + 1] |= overflow_mask & (uint64_t(x) >> bits_available);

    // increment body size on qword overflow
    meta.body_size += overflow_mask & 1;

    // increment body cursor by bits serialized
    meta.body_cursor += bits;
}

template <typename T>
bool PacketBuffer<T>::DeserializeBool()
{
    return serialize<bool, 1>();
}

template <typename T>
uint8_t PacketBuffer<T>::DeserializeUInt8()
{
    return serialize<uint8_t, 8>();
}

template <typename T>
uint16_t PacketBuffer<T>::DeserializeUInt16()
{
    return serialize<uint16_t, 16>();
}

template <typename T>
uint32_t PacketBuffer<T>::DeserializeUInt32()
{
    return serialize<uint32_t, 32>();
}

template <typename T>
uint64_t PacketBuffer<T>::DeserializeUInt64()
{
    return serialize<uint64_t, 64>();
}

template <typename T>
KeyBuffer PacketBuffer<T>::DeserializeKeyBuffer()
{
    KeyBuffer buffer;
    for (auto &qword : buffer)
    {
        qword = DeserializeUInt64();
    }

    return buffer;
}
