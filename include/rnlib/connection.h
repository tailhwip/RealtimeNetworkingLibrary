#ifndef RN_CONNECTION_H
#define RN_CONNECTION_H

#include "cryptography.h"
#include "handshake.h"
#include "packet.h"

#ifdef __cplusplus
extern "C"
{
#endif

enum RnConnectionReadResult
{
    RN_CONNECTION_READ_AVAILABLE,
    RN_CONNECTION_READ_EMPTY,
    RN_CONNECTION_READ_HANDSHAKE,
    RN_CONNECTION_READ_ERROR_SEQUENCE,
    RN_CONNECTION_READ_ERROR_CONTEXT,
    RN_CONNECTION_READ_ERROR_VERIFY,
};

enum RnConnectionWriteResult
{
    RN_CONNECTION_WRITE_OK,
    RN_CONNECTION_WRITE_ERROR_CONTEXT,
    RN_CONNECTION_WRITE_ERROR_AUTHENTICATE,
};

#define RN_CONNECTION_DECL(TYPE, IP, BUFFER)                                                       \
    typedef struct RnConnection##TYPE##IP RnConnection##TYPE##IP;                                  \
                                                                                                   \
    void rnConnectionEstablishHandshake(RnConnection##TYPE##IP *connection);                       \
                                                                                                   \
    enum RnConnectionReadResult rnConnectionReadPacket(                                            \
          RnConnection##TYPE##IP *connection, RnPacketBuffer##BUFFER *buffer);                     \
                                                                                                   \
    enum RnConnectionWriteResult rnConnectionWritePacket(                                          \
          RnConnection##TYPE##IP *connection, RnPacketBuffer##BUFFER *buffer);

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
RN_CONNECTION_DECL(Authenticated, IPv4, Secure)
RN_CONNECTION_DECL(Authenticated, IPv6, Secure)

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
RN_CONNECTION_DECL(Encrypted, IPv4, Secure)
RN_CONNECTION_DECL(Encrypted, IPv6, Secure)

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
RN_CONNECTION_DECL(Insecure, IPv4, Insecure)
RN_CONNECTION_DECL(Insecure, IPv6, Insecure)

#undef RN_CONNECTION_DECL

#ifdef __cplusplus
}
#endif

#endif // RN_CONNECTION_H
