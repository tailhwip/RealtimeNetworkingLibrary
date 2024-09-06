#include "../include/rnlib/packet.h"

using namespace rn;

PacketBuffer::PacketBuffer(uint64_t *body, uint_fast8_t body_size)
    : body { body }, body_size { body_size }
{
}

template <typename T>
inline void PacketBuffer::serialize(T data, uint_fast8_t bit_count)
{
    int_fast8_t bits_available = 64 - scratch_cursor;
    uint64_t overflow_mask = -uint64_t(bit_count > bits_available);

    scratch |= uint64_t(data) << scratch_cursor;

    // flush scratch on overflow, keep at 0 otherwise
    body[body_cursor] = overflow_mask & scratch;

    // increment index on overflow, increment by 0 otherwise
    body_cursor += overflow_mask & 1;

    // set scratch to remainder on overflow, keep as is otherwise
    scratch += overflow_mask & ((uint64_t(data) >> bits_available) - scratch);

    // adjust cursor to remainder bit count on overflow
    scratch_cursor -= overflow_mask & (64 - bit_count);

    // add written bits to cursor otherwise
    scratch_cursor += ~overflow_mask & bit_count;
}

void PacketBuffer::serialize_bool(bool b)
{
    serialize<bool>(b, 1);
}

void PacketBuffer::serialize_uint8(uint8_t x)
{
    serialize<uint8_t>(x, sizeof(uint8_t) * 8);
}

void PacketBuffer::serialize_uint16(uint16_t x)
{
    serialize<uint16_t>(x, sizeof(uint16_t) * 8);
}

void PacketBuffer::serialize_uint32(uint32_t x)
{
    serialize<uint32_t>(x, sizeof(uint32_t) * 8);
}

void PacketBuffer::serialize_uint64(uint64_t x)
{
    serialize<uint64_t>(x, sizeof(uint64_t) * 8);
}

SecurePacketBuffer::SecurePacketBuffer(uint8_t packet_type)
    : PacketBuffer((uint64_t *)&body, qwords)
{
    header.protocol = RN_PROTOCOL_ID;
    header.type = packet_type;
}

InsecurePacketBuffer::InsecurePacketBuffer(uint8_t packet_type)
    : PacketBuffer((uint64_t *)&body, qwords)
{
    header.protocol = RN_PROTOCOL_ID;
    header.type = packet_type;
}

uint16_t PacketSequence::operator++()
{
    ++number;

    uint16_t overflow_mask = -uint16_t(number == UINT16_MAX);

    // increment generation on overflow, increment by 0 otherwise
    generation += overflow_mask & 1;

    // reset number to 0 on overflow, keep as is otherwise
    number -= overflow_mask & number;

    return number;
}

std::optional<PacketSequence> PacketSequence::operator<<(uint16_t number)
{
    if (number >= this->number)
    {
        if (number - this->number < 1'024)
        {
            return PacketSequence { number, this->generation };
        }
    }

    if (this->number - number > 32'768)
    {
        return PacketSequence { number, uint16_t(this->generation + 1) };
    }

    return std::nullopt;
}
