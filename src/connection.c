#include "../include/rnlib/connection.h"
#include "../include/rnlib/handshake.h"

#include <time.h>

#ifdef _WIN32
    #define SODIUM_STATIC 1
    #define SODIUM_EXPORT
#endif

#include <sodium.h>

union RnPacketSequence
{
    struct RnPacketSequenceCounter
    {
        uint16_t generation;
        uint16_t number;

    } counter;

    /**
     * Secure packets require a unique nonce for each packet created during a
     * single session. These need not be private, thus this union combines the
     * sequence number and generation to create an incrementing sequence
     * sufficiently large for most use cases.
     *
     * Technically, this limits a secure session to 2^32 - 1 packets each
     * way before their cryptographic integrity breaks.
     */
    uint32_t nonce;
};

/**
 * Increments counter by 1 and increments its generation if this number has
 * reached its maximum value.
 */
RnPacketSequenceCounter rnPacketSequenceIncrement(RnPacketSequenceCounter counter)
{
    ++counter.number;

    uint16_t overflow_mask = -uint16_t(counter.number == UINT16_MAX);

    // increment generation on overflow, increment by 0 otherwise
    counter.generation += overflow_mask & 1;

    // reset number to 0 on overflow, keep as is otherwise
    counter.number -= overflow_mask & counter.number;

    return counter;
}

/**
 * Attempts to advance `counter` to `number`, returning a 0 counter if `number`
 * is too out-of-sync with `counter`.
 */
RnPacketSequenceCounter rnPacketSequenceAdvance(RnPacketSequenceCounter counter, uint16_t number)
{
    if (number >= counter.number)
    {
        if (number - counter.number < 1024)
        {
            return PacketSequenceCounter { counter.generation, number };
        }
    }

    if (counter.number - number > 32768)
    {
        return PacketSequenceCounter { counter.generation + 1, number };
    }

    return RnPacketSequenceCounter { 0, 0 };
}

struct RnConnectionStreamSecure
{
    RnKeyBuffer key;           // 32
    RnPacketSequence sequence; // 4
    uint8_t idle_since;        // 1

    RnAddressIPv4 address; // 6
};

struct RnConnectionAuthenticatedIngress
{
    RnKeyBuffer key;           // 32
    RnPacketSequence sequence; // 4
};

struct RnConnectionAuthenticated
{
    RnAddressIPv4 address;
    RnKeyPair keys;
    RnHandshakeIPv4 handshake;

    RnPacketSequence sequence_egress;
    RnPacketSequence sequence_ingress;
};

struct RnConnectionPoolAuthenticated
{
    RnSocketIPv4 socket;

    RnHandshakeIPv4 handshake[UINT16_MAX];
    RnConnectionAuthenticatedEgress egress[UINT16_MAX];
    RnConnectionAuthenticatedIngress ingress[UINT16_MAX];
};

#define RN_CONNECTION_SECURE_IMPL(TYPE, IP)                                                        \
    struct RnConnection##TYPE##IP                                                                  \
    {                                                                                              \
        RnSocket##IP socket;                                                                       \
        RnAddress##IP address;                                                                     \
        RnHandshakeSecure handshake;                                                               \
        RnPacketSequence incoming;                                                                 \
        RnPacketSequence outgoing;                                                                 \
                                                                                                   \
        RnSessionKeys keys;                                                                        \
    };

RN_CONNECTION_SECURE_IMPL(Authenticated, IPv4)
RN_CONNECTION_SECURE_IMPL(Authenticated, IPv6)

RN_CONNECTION_SECURE_IMPL(Encrypted, IPv4)
RN_CONNECTION_SECURE_IMPL(Encrypted, IPv6)

#define RN_CONNECTION_INSECURE_IMPL(IP)                                                            \
    struct RnConnectionInsecure##IP                                                                \
    {                                                                                              \
        RnSocket##IP socket;                                                                       \
        RnAddress##IP address;                                                                     \
        RnHandshakeInsecure handshake;                                                             \
        RnPacketSequence incoming;                                                                 \
        RnPacketSequence outgoing;                                                                 \
    };

RN_CONNECTION_INSECURE_IMPL(IPv4)
RN_CONNECTION_INSECURE_IMPL(IPv6)

#define RN_CONNECTION_IMPL(TYPE, IP, BUFFER)                                                       \
    enum RnReadPacketResult rnConnectionReadPacket(                                                \
          RnConnection##TYPE##IP *connection, RnPacketBuffer##BUFFER *buffer)                      \
    {                                                                                              \
        RnPacketSequence sequence = rnPacketSequenceAdvance(                                       \
              connection->incoming, buffer->header.sequence);                                      \
        if (sequence.nonce == 0)                                                                   \
        {                                                                                          \
            return RN_CONNECTION_READ_ERROR_SEQUENCE;                                              \
        }                                                                                          \
                                                                                                   \
        int read_result = rnConnectionReadPacket##TYPE(connection, buffer);                        \
        if (read_result == RN_CONNECTION_READ_AVAILABLE)                                           \
        {                                                                                          \
            connection->incoming = sequence;                                                       \
            connection->idle_since = time(0) % UINT8_MAX;                                          \
                                                                                                   \
            int handshake_status = rnHandshakeReadPacket(packet);                                  \
            if (handshake_status != RN_HANDSHAKE_STEP_SERVER_CONNECTED)                            \
            {                                                                                      \
                return RN_CONNECTION_READ_IGNORE;                                                  \
            }                                                                                      \
        }                                                                                          \
                                                                                                   \
        return read_result;                                                                        \
    }                                                                                              \
                                                                                                   \
    enum RnConnectionWriteResult rnConnectionWritePacket(                                          \
          RnConnection##TYPE##IP *connection, RnPacketBuffer##BUFFER *buffer)                      \
    {                                                                                              \
        RnPacketSequence next_sequence = rnPacketBufferSequenceIncrement(sequence);                \
        int write_result = writer.WritePacket(handshake, sequence, packet);                        \
        if (write_result == RN_OK)                                                                 \
        {                                                                                          \
            sequence = next_sequence;                                                              \
            idle_since = time(0) % UINT8_MAX;                                                      \
            \                                                                                      \
        }                                                                                          \
                                                                                                   \
        return write_result;                                                                       \
    }

RN_CONNECTION_IMPL(Authenticated, IPv4, Secure)
RN_CONNECTION_IMPL(Authenticated, IPv6, Secure)

RN_CONNECTION_IMPL(Encrypted, IPv4, Secure)
RN_CONNECTION_IMPL(Encrypted, IPv6, Secure)

RN_CONNECTION_IMPL(Insecure, IPv4, Insecure)
RN_CONNECTION_IMPL(Insecure, IPv6, Insecure)

enum RnConnectionReadResult rxConnectionPreProcessIngressAuthenticated(
      const RnPacketBufferSecure *buffer, RnPacketBufferMetaData meta, const RnKeyBuffer *master,
      uint64_t context, uint64_t nonce)
{
    RnKeyBuffer key;
    int key_result = crypto_kdf_derive_from_key(key, sizeof key, nonce, context, master);
    if (key_result != RN_OK)
    {
        return RN_CONNECTION_READ_ERROR_CONTEXT;
    }

    size_t data_size = sizeof buffer.header + meta.body_size;
    int crypto_result = crypto_onetimeauth_verify(buffer.auth, buffer.header, data_size, key);
    if (crypto_result != RN_OK)
    {
        return RN_CONNECTION_READ_ERROR_VERIFY;
    }

    return RN_OK;
}

enum RnConnectionWriteResult rxConnectionPreProcessEgressAuthenticated(
      RnPacketBufferSecure *buffer, RnPacketBufferMetaData meta, const RnKeyBuffer *master,
      uint64_t context, uint64_t nonce)
{
    RnKeyBuffer key;
    int key_result = crypto_kdf_derive_from_key(key, sizeof key, nonce, context, master);
    if (key_result != RN_OK)
    {
        return RN_CONNECTION_WRITE_ERROR_CONTEXT;
    }

    size_t data_size = sizeof buffer.header + meta.body_size;
    int crypto_result = crypto_onetimeauth(buffer.auth, buffer.header, data_size, key);
    if (crypto_result != RN_OK)
    {
        return RN_CONNECTION_WRITE_ERROR_AUTHENTICATE;
    }

    return RN_OK;
}

enum RnConnectionReadResult rxConnectionPreProcessIngressEncrypted(
      RnPacketBufferSecure *buffer, RnPacketBufferMetaData meta, const RnKeyBuffer *master,
      uint64_t context, uint64_t nonce)
{
    size_t data_size = sizeof buffer.header + meta.body_size;
    uint64_t nonce_buffer[3] = { context, 0, nonce };
    int result = crypto_box_open_easy_afternm(
          buffer.header, buffer.auth, data_size, nonce_buffer, key);
    if (result != RN_OK)
    {
        return RN_CONNECTION_READ_ERROR_VERIFY;
    }

    return RN_OK;
}

enum RnConnectionWriteResult rxConnectionPreProcessEgressEncrypted(
      RnPacketBufferSecure *buffer, RnPacketBufferMetaData meta, const RnKeyBuffer *master,
      uint64_t context, uint64_t nonce)
{
    size_t data_size = sizeof buffer.header + meta.body_size * 8;
    uint64_t nonce_buffer[3] = { context, 0, nonce };
    int result = crypto_box_easy_afternm(buffer.auth, buffer.header, data_size, nonce_buffer, key);
    if (result != RN_OK)
    {
        return WritePacketResult::ERR_AUTHENTICATE;
    }

    return WritePacketResult::SUCCESS;
}

enum RnConnectionReadResult rxConnectionPreProcessIngressInsecure(
      RnPacketBufferInsecure *buffer, RnPacketBufferMetaData meta, uint64_t salt)
{
    if (buffer.salt != salt)
    {
        return RN_CONNECTION_READ_ERROR_CONTEXT;
    }

    // TODO remove
    // uint16_t checksum = crc_16(buff.header, buff.byte_size());
    // if (buff.tag.crc16 != checksum)
    // {
    //     return ReadPacketResult::ERR_VERIFY;
    // }

    return RN_OK;
}

// TODO remove
// WritePacketResult InsecurePacketWriter::WritePacket(_Inout_
// InsecurePacketBuffer &buff)
// {
//     buff.header.sequence = ++sequence;
//     buff.tag.crc16 = crc_16(buff.header, buff.byte_size());

//     return TagPacket(buff);
// }

enum RnConnectionWriteResult rxConnectionPreProcessEgressInsecure(
      RnPacketBufferInsecure *buffer, RnPacketBufferMetaData meta, uint64_t salt)
{
    buffer.salt = data;
    return RN_OK;
}
