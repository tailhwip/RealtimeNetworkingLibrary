#ifndef RN_HANDSHAKE_H
#define RN_HANDSHAKE_H

#include "cryptography.h"
#include "packet.h"

#include <bool.h>
#include <stdint.h>

#define RN_HANDSHAKE_PACKET_TYPE UINT16_MAX

#ifdef __cplusplus
extern "C"
{
#endif

enum RnHandshakeStep
{
    RN_HANDSHAKE_DISCONNECTED,

    RN_HANDSHAKE_CLIENT_CONNECT,
    RN_HANDSHAKE_SERVER_CHALLENGE,
    RN_HANDSHAKE_CLIENT_RESPONSE,
    RN_HANDSHAKE_SERVER_CONNECTED,

    RN_HANDSHAKE_CONNECTED,
};

#define RN_HANDSHAKE_DECL(TYPE)                                                \
    typedef struct RnHandshake##TYPE RnHandshake##TYPE;                        \
                                                                               \
    RnHandshake##TYPE rnHandshakeCreate##TYPE();                               \
                                                                               \
    bool rnHandshakeReadPacketClient(                                          \
          RnHandshake##TYPE *handshake, RnPacketBuffer##TYPE *buffer);         \
    bool rnHandshakeWritePacketClient(                                         \
          RnHandshake##TYPE *handshake, OUT RnPacketBuffer##TYPE *buffer);     \
                                                                               \
    bool rnHandshakeReadPacketServer(                                          \
          RnHandshake##TYPE *handshake, RnPacketBuffer##TYPE *buffer);         \
    bool rnHandshakeWritePacketServer(                                         \
          RnHandshake##TYPE *handshake, OUT RnPacketBuffer##TYPE *buffer);

RN_HANDSHAKE_DECL(Secure)
RN_HANDSHAKE_DECL(Insecure)

#ifdef __cplusplus
}
#endif

#endif // RN_HANDSHAKE_H
