#include "../include/rnlib/connection.h"

#include <ctime>

#ifdef _WIN32
    #define SODIUM_STATIC 1
    #define SODIUM_EXPORT
#endif

#include <checksum.h>
#include <sodium.h>

using namespace rn;

ReadAuthenticatedPacketResult AuthenticatedPacketReader::ReadPacket(
        const SecurePacketBuffer &buff)
{
    auto next_sequence = this->sequence << buff.header.sequence;
    if (!next_sequence.has_value())
    {
        return ReadAuthenticatedPacketResult::REJECT;
    }

    auto key = master_key.DeriveSingleUse(context, next_sequence->nonce);
    if (!key.has_value())
    {
        return ReadAuthenticatedPacketResult::FAILURE_CONTEXT;
    }

    if (crypto_onetimeauth_verify(buff.auth, buff.header, buff.meta.byte_size, *key) != 0)
    {
        return ReadAuthenticatedPacketResult::FAILURE_AUTHENTICATE;
    }

    sequence = *next_sequence;
    idle_since = time(0) % UINT8_MAX;
    return ReadAuthenticatedPacketResult::ACCEPT;
}

WriteAuthenticatedPacketResult AuthenticatedPacketWriter::WritePacket(
        _Inout_ SecurePacketBuffer &buff)
{
    buff.header.sequence = ++sequence;
    auto key = master_key.DeriveSingleUse(context, sequence.nonce);
    if (!key.has_value())
    {
        return WriteAuthenticatedPacketResult::FAILURE_CONTEXT;
    }

    if (crypto_onetimeauth(buff.auth, buff.header, buff.meta.byte_size, *key) != 0)
    {
        return WriteAuthenticatedPacketResult::FAILURE_AUTHENTICATE;
    }

    idle_since = time(0) % UINT8_MAX;
    return WriteAuthenticatedPacketResult::SUCCESS;
}

ReadEncryptedPacketResult EncryptedPacketReader::ReadPacket(
        _Inout_ SecurePacketBuffer &buff)
{
    auto next_sequence = this->sequence << buff.header.sequence;
    if (!next_sequence.has_value())
    {
        return ReadEncryptedPacketResult::REJECT;
    }

    static NonceBuffer nonce;
    nonce = next_sequence->nonce;

    size_t size = buff.meta.byte_size;
    if (crypto_box_open_easy_afternm(buff.header, buff.auth, size, nonce, key) != 0)
    {
        return ReadEncryptedPacketResult::FAILURE_DECRYPT;
    }

    sequence = *next_sequence;
    idle_since = time(0) % UINT8_MAX;
    return ReadEncryptedPacketResult::ACCEPT;
}

WriteEncryptedPacketResult EncryptedPacketWriter::WritePacket(
        _Inout_ SecurePacketBuffer &buff)
{
    buff.header.sequence = ++sequence;

    static NonceBuffer nonce;
    nonce = sequence.nonce;

    size_t size = buff.meta.byte_size;
    if (crypto_box_easy_afternm(buff.auth, buff.header, size, nonce, key) != 0)
    {
        return WriteEncryptedPacketResult::FAILURE_ENCRYPT;
    }

    idle_since = time(0) % UINT8_MAX;
    return WriteEncryptedPacketResult::SUCCESS;
}

ReadInsecurePacketResult InsecurePacketReader::ReadPacket(
        const InsecurePacketBuffer &buff)
{
    // TODO check buff.checksum.salt_lower
    if (buff.checksum.salt_upper != salt)
    {
        return ReadInsecurePacketResult::FAILURE_CONTEXT;
    }

    auto next_sequence = this->sequence << buff.header.sequence;
    if (!next_sequence.has_value())
    {
        return ReadInsecurePacketResult::REJECT;
    }

    uint16_t crc16 = crc_16(buff.header, buff.meta.byte_size);
    if (buff.checksum.crc16 != crc16)
    {
        return ReadInsecurePacketResult::FAILURE_CHECKSUM;
    }

    sequence = *next_sequence;
    idle_since = time(0) % UINT8_MAX;
    return ReadInsecurePacketResult::ACCEPT;
}

WriteInsecurePacketResult InsecurePacketWriter::WritePacket(
        _Inout_ InsecurePacketBuffer &buff)
{
    buff.header.sequence = ++sequence;

    buff.checksum.crc16 = crc_16(buff.header, buff.meta.byte_size);

    return TagPacket(buff);
}

WriteInsecurePacketResult InsecurePacketWriter::TagPacket(
        _Inout_ InsecurePacketBuffer &buff)
{
    // TODO set buff.checksum.salt_lower
    buff.checksum.salt_upper = salt;

    idle_since = time(0) % UINT8_MAX;
    return WriteInsecurePacketResult::SUCCESS;
}
