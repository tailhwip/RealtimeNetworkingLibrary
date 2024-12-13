#include "../include/rnlib/cryptography.h"

#ifdef _WIN32
    #define SODIUM_STATIC 1
    #define SODIUM_EXPORT
#endif

#include <sodium.h>

int rnKeyPairGenerateEphemeral(OUT RnKeyPair *out)
{
    return crypto_kx_keypair(out->pub, out->sec);
}

int rnKeyPairComputeSessionClient(
      const KeyPair *client, const KeyBuffer *server_pub, OUT RnKeyPair *out)
{
    return crypto_kx_client_session_keys(out->sec, out->pub, client->pub, client->sec, server_pub);
}

int rnKeyPairComputeSessionServer(
      const KeyPair *server, const KeyBuffer *client_pub, OUT RnKeyPair *out)
{
    return crypto_kx_client_session_keys(out->sec, out->pub, server->pub, server->sec, client_pub);
}
