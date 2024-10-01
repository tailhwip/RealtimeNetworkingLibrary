#pragma once

#include "cryptography.h"
#include "handshake.h"
#include "packet.h"

#include <optional>

namespace rn
{

enum class ReadPacketResult
{
    ACCEPT,
    IGNORE,
    ERR_SEQUENCE,
    ERR_CONTEXT,
    ERR_VERIFY,
};

enum class WritePacketResult
{
    SUCCESS,
    ERR_CONTEXT,
    ERR_AUTHENTICATE,
};

template <
    typename ADDRESS_T,
    typename BUFFER_T,
    typename READER_T,
    typename WRITER_T,
    typename HANDSHAKE_T>
class Connection
{
public:
    using buffer_t = BUFFER_T;

private:
    ADDRESS_T address;
    READER_T reader;
    WRITER_T writer;
    HANDSHAKE_T handshake;

public:
    Connection(const ADDRESS_T &address, const void *data);

    inline const ADDRESS_T &GetAddress() const { return address; }

    ReadPacketResult ReadPacket(_Inout_ buffer_t &packet);
    WritePacketResult WritePacket(_Inout_ buffer_t &packet);
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

class AuthenticatedPacketReader
{
private:
    KeyBuffer master_key;
    PacketSequence sequence;
    uint8_t idle_since;

public:
    AuthenticatedPacketReader(const KeyBuffer &master_key);

    ReadPacketResult ReadPacket(
        const SecureHandshake &handshake,
        const SecurePacketBuffer &packet);
};

class AuthenticatedPacketWriter
{
private:
    KeyBuffer master_key;
    PacketSequence sequence;
    uint8_t idle_since;

public:
    AuthenticatedPacketWriter(const KeyBuffer &master_key);

    WritePacketResult WritePacket(
        const SecureHandshake &handshake,
        _Inout_ SecurePacketBuffer &packet);
};

/**
 * Authenticated connections tag their otherwise plain text packets with 16 byte
 * Poly1305 authentication tags. The cryptographic keys used are securely
 * exchanged by means of sharing public keys and then derived as single-use keys
 * from shared session keys constructed using said public keys and corresponding
 * local secret key.
 *
 * Authenticated connections are intended to be used for high-throughput traffic
 * that doesn't require privacy but needs to be protected from MITM attacks.
 */
template <typename ADDRESS_T>
using AuthenticatedConnection = Connection<
    ADDRESS_T,
    SecurePacketBuffer,
    AuthenticatedPacketReader,
    AuthenticatedPacketWriter,
    SecureHandshake>;

class EncryptedPacketReader
{
private:
    KeyBuffer key;
    PacketSequence sequence;
    uint8_t idle_since;

public:
    ReadPacketResult ReadPacket(
        const SecureHandshake &handshake,
        _Inout_ SecurePacketBuffer &packet);
};

class EncryptedPacketWriter
{
private:
    KeyBuffer key;
    PacketSequence sequence;
    uint8_t idle_since;

public:
    WritePacketResult WritePacket(
        const SecureHandshake &handshake,
        _Inout_ SecurePacketBuffer &packet);
};

/**
 * Encrypted connections completely obfuscate their packets using XSalsa20
 * encryption and are tagged with 16 byte Poly1305 authentication tags as used
 * by authenticated connections. The cryptographic keys used are securely
 * exchanged by means of sharing public keys and then deriving a set of shared
 * session keys from said public keys and corresponding local secret key.
 *
 * Encrypted connections are intended to be used for low-throughput traffic
 * that requires both privacy and reliability.
 *
 * TODO impl packet acknowledgement and replay
 */
template <typename ADDRESS_T>
using EncryptedConnection = Connection<
    ADDRESS_T,
    SecurePacketBuffer,
    EncryptedPacketReader,
    EncryptedPacketWriter,
    SecureHandshake>;

class InsecurePacketReader
{
private:
    PacketSequence sequence;
    uint8_t idle_since;

public:
    ReadPacketResult ReadPacket(
        const InsecureHandshake &handshake,
        const InsecurePacketBuffer &packet);
};

class InsecurePacketWriter
{
private:
    PacketSequence sequence;
    uint8_t idle_since;

public:
    WritePacketResult WritePacket(
        const InsecureHandshake &handshake,
        _Inout_ InsecurePacketBuffer &packet);
};

/**
 * ! Insecure connections are vulnerable to MITM attacks.
 *
 * Insecure connections solely tag their plain text packets with a previously in
 * the open agreed upon 64 bit salt. This reasonably protects against random
 * impersonation attacks but otherwise leaves the connection completely open to
 * MITM attacks. The salt is renegotiated every 2^16 - 1 packets to provide some
 * level of protection against brute-force attacks.
 *
 * Insecure connections are intended to be used for very high-throughput traffic
 * that doesn't require privacy and can, to a certain degree, tolerate a
 * vulnerability to MITM attacks.
 *
 * ! This connection type is mostly a 'how fast can it go' case study and is
 * best avoided in production environments.
 *
 * Example use cases include an authoritative MMO server sending 30 movement
 * packets per second to 30,000 players. Insecure connections do not require any
 * recomputation to send an already established packet to another client. It
 * slaps on the next salt and gives it to the socket.
 */
template <typename ADDRESS_T>
using InsecureConnection = Connection<
    ADDRESS_T,
    InsecurePacketBuffer,
    InsecurePacketReader,
    InsecurePacketWriter,
    InsecureHandshake>;

} // namespace rn
