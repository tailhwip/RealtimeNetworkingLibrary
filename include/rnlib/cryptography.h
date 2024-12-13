#ifndef RN_CRYPTOGRAPHY_H
#define RN_CRYPTOGRAPHY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef uint8_t RnKeyBuffer[32] __attribute__((aligned(8)));

struct RnKeyPair
{
    RnKeyBuffer pub;
    RnKeyBuffer sec;
};

typedef struct RnKeyPair RnKeyPair;

/**
 * Generates an ephemeral public/secret key pair meant to be destroyed at
 * the end of the session.
 */
int rnKeyPairGenerateEphemeral(OUT RnKeyPair *out);

/**
 * Computes a shared session key pair for use by the client from the provided
 * client key pair and server public key.
 */
int rnKeyPairComputeSessionClient(
      const RnKeyPair *client_keys, const RnKeyBuffer *server_pubkey, OUT RnKeyPair *out);

/**
 * Computes a shared session key pair for use by the server from the provided
 * server key pair and client public key.
 */
int rnKeyPairComputeSessionServer(
      const KeyPair &server_keys, const KeyBuffer &client_pubkey, OUT RnKeyPair *out);

#ifdef __cplusplus
}
#endif

#endif // RN_CRYPTOGRAPHY_H
