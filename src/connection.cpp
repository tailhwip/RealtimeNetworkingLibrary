#include "../include/rnlib/connection.h"

#include "../include/rnlib/handshake.h"

#include <ctime>

#ifdef _WIN32
    #define SODIUM_STATIC 1
    #define SODIUM_EXPORT
#endif

#include <sodium.h>

using namespace rn;

template <typename T, typename BUFFER_T, typename V, typename W, typename X>
ReadPacketResult Connection<T, BUFFER_T, V, W, X>::ReadPacket(_Inout_ BUFFER_T &packet)
{
    ReadPacketResult read_result = reader.ReadPacket(handshake, packet);
    if (read_result == ReadPacketResult::ACCEPT)
    {
        HandshakeStatus handshake_status = handshake.ReadPacket(packet);
        if (handshake_status == HandshakeStatus::CONNECTED)
        {
            return ReadPacketResult::ACCEPT;
        }

        return ReadPacketResult::IGNORE;
    }

    return read_result;
}

template <typename T, typename BUFFER_T, typename V, typename W, typename X>
WritePacketResult Connection<T, BUFFER_T, V, W, X>::WritePacket(_Inout_ BUFFER_T &packet)
{
    return writer.WritePacket(handshake, packet);
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

ReadPacketResult AuthenticatedPacketReader::ReadPacket(
        const SecureHandshake &handshake, const SecurePacketBuffer &buffer)
{
    auto next_sequence = this->sequence << buffer.header.sequence;
    if (!next_sequence.has_value())
    {
        return ReadPacketResult::ERR_SEQUENCE;
    }

    auto key = master_key.DeriveSingleUse(handshake.data, next_sequence->nonce);
    if (!key.has_value())
    {
        return ReadPacketResult::ERR_CONTEXT;
    }

    size_t length = buffer.byte_size();
    if (crypto_onetimeauth_verify(buffer.tag, buffer.header, length, *key) != 0)
    {
        return ReadPacketResult::ERR_VERIFY;
    }

    sequence = *next_sequence;
    idle_since = time(0) % UINT8_MAX;
    return ReadPacketResult::ACCEPT;
}

WritePacketResult AuthenticatedPacketWriter::WritePacket(
        const SecureHandshake &handshake, _Inout_ SecurePacketBuffer &buffer)
{
    buffer.header.sequence = ++sequence;
    auto key = master_key.DeriveSingleUse(handshake.data, sequence.nonce);
    if (!key.has_value())
    {
        return WritePacketResult::ERR_CONTEXT;
    }

    size_t length = buffer.byte_size();
    if (crypto_onetimeauth(buffer.tag, buffer.header, length, *key) != 0)
    {
        return WritePacketResult::ERR_AUTHENTICATE;
    }

    // TODO sequence might be incremented if returned earlier
    idle_since = time(0) % UINT8_MAX;
    return WritePacketResult::SUCCESS;
}

ReadPacketResult EncryptedPacketReader::ReadPacket(
        const SecureHandshake &handshake, _Inout_ SecurePacketBuffer &buff)
{
    auto next_sequence = this->sequence << buff.header.sequence;
    if (!next_sequence.has_value())
    {
        return ReadPacketResult::ERR_SEQUENCE;
    }

    static NonceBuffer nonce;
    nonce = next_sequence->nonce;

    size_t length = buff.byte_size();
    if (crypto_box_open_easy_afternm(buff.header, buff.tag, length, nonce, key) != 0)
    {
        return ReadPacketResult::ERR_VERIFY;
    }

    sequence = *next_sequence;
    idle_since = time(0) % UINT8_MAX;
    return ReadPacketResult::ACCEPT;
}

WritePacketResult EncryptedPacketWriter::WritePacket(
        const SecureHandshake &handshake, _Inout_ SecurePacketBuffer &buff)
{
    buff.header.sequence = ++sequence;

    static NonceBuffer nonce;
    nonce = sequence.nonce;

    size_t length = buff.byte_size();
    if (crypto_box_easy_afternm(buff.tag, buff.header, length, nonce, key) != 0)
    {
        return WritePacketResult::ERR_AUTHENTICATE;
    }

    idle_since = time(0) % UINT8_MAX;
    return WritePacketResult::SUCCESS;
}

ReadPacketResult InsecurePacketReader::ReadPacket(
        const InsecureHandshake &handshake, const InsecurePacketBuffer &buff)
{
    if (buff.tag.salt != handshake.data)
    {
        return ReadPacketResult::ERR_CONTEXT;
    }

    auto next_sequence = this->sequence << buff.header.sequence;
    if (!next_sequence.has_value())
    {
        return ReadPacketResult::ERR_SEQUENCE;
    }

    // TODO remove
    // uint16_t checksum = crc_16(buff.header, buff.byte_size());
    // if (buff.tag.crc16 != checksum)
    // {
    //     return ReadPacketResult::ERR_VERIFY;
    // }

    sequence = *next_sequence;
    idle_since = time(0) % UINT8_MAX;
    return ReadPacketResult::ACCEPT;
}

// TODO remove
// WritePacketResult InsecurePacketWriter::WritePacket(_Inout_ InsecurePacketBuffer &buff)
// {
//     buff.header.sequence = ++sequence;
//     buff.tag.crc16 = crc_16(buff.header, buff.byte_size());

//     return TagPacket(buff);
// }

WritePacketResult InsecurePacketWriter::WritePacket(
        const InsecureHandshake &handshake, _Inout_ InsecurePacketBuffer &buff)
{
    buff.tag.salt = handshake.data;

    idle_since = time(0) % UINT8_MAX;
    return WritePacketResult::SUCCESS;
}
