#pragma once

#include "cryptography.h"
#include "packet.h"

#include <cstdint>

namespace rn
{

enum class ConnectionState
{
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
};

class AuthenticatedPacketReader
{
    KeyBuffer master_key;
    uint64_t context;
    PacketSequence sequence;
    uint8_t idle_since;

    enum class ReadAuthenticatedPacketResult ReadPacket(
        const SecurePacketBuffer &packet);
};

enum class ReadAuthenticatedPacketResult
{
    ACCEPT,
    REJECT,
    FAILURE_CONTEXT,
    FAILURE_AUTHENTICATE,
};

class AuthenticatedPacketWriter
{
    KeyBuffer master_key;
    uint64_t context;
    PacketSequence sequence;
    uint8_t idle_since;

    enum class WriteAuthenticatedPacketResult WritePacket(
        _Inout_ SecurePacketBuffer &packet);
};

enum class WriteAuthenticatedPacketResult
{
    SUCCESS,
    FAILURE_CONTEXT,
    FAILURE_AUTHENTICATE,
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
template <typename IPvT>
class AuthenticatedConnection
{
    IPvT address;
    AuthenticatedPacketReader reader;
    AuthenticatedPacketWriter writer;
};

class EncryptedPacketReader
{
    KeyBuffer key;
    uint64_t context;
    PacketSequence sequence;
    uint8_t idle_since;

    enum class ReadEncryptedPacketResult ReadPacket(
        _Inout_ SecurePacketBuffer &packet);
};

enum class ReadEncryptedPacketResult
{
    ACCEPT,
    REJECT,
    FAILURE_DECRYPT,
};

class EncryptedPacketWriter
{
    KeyBuffer key;
    uint64_t context;
    PacketSequence sequence;
    uint8_t idle_since;

    enum class WriteEncryptedPacketResult WritePacket(
        _Inout_ SecurePacketBuffer &packet);
};

enum class WriteEncryptedPacketResult
{
    SUCCESS,
    FAILURE_ENCRYPT,
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
template <typename IPvT>
class EncryptedConnection
{
    IPvT address;
    EncryptedPacketReader reader;
    EncryptedPacketWriter writer;
};

class InsecurePacketReader
{
    uint32_t salt;
    PacketSequence sequence;
    uint8_t idle_since;

    enum class ReadInsecurePacketResult ReadPacket(
        const InsecurePacketBuffer &packet);
};

enum class ReadInsecurePacketResult
{
    ACCEPT,
    REJECT,
    FAILURE_CONTEXT,
    FAILURE_CHECKSUM,
};

class InsecurePacketWriter
{
    uint32_t salt;
    PacketSequence sequence;
    uint8_t idle_since;

    enum class WriteInsecurePacketResult WritePacket(
        _Inout_ InsecurePacketBuffer &packet);

    enum class WriteInsecurePacketResult TagPacket(
        _Inout_ InsecurePacketBuffer &packet);
};

enum class WriteInsecurePacketResult
{
    SUCCESS,
};

/**
 * ! Insecure connections are vulnerable to MITM attacks.
 *
 * Insecure connections solely tag their plain text packets with a previously in
 * the open agreed upon 32 bit salt. This somewhat protects against random
 * impersonation attempts but otherwise leaves the connection completely open to
 * MITM attacks. The salt is renegotiated every 2^16 - 1 packets to provide some
 * level of protection against brute-force attacks.
 *
 * Insecure connections are intended to be used for very high-throughput traffic
 * that doesn't require privacy or protection from MITM attacks.
 *
 * ! This connection type is mostly a 'how fast can it go' case study and is
 * best avoided in production environments.
 *
 * Example use cases include an authoritative MMO server sending 30 movement
 * packets per second to 30,000 players. Insecure connections do not require any
 * recomputation to send an already established packet to another client. It
 * slaps on the next salt and gives it to the socket.
 */
template <typename IPvT>
class InsecureConnection
{
    IPvT address;
    InsecurePacketReader reader;
    InsecurePacketWriter writer;
};

} // namespace rn
