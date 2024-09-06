#include "../include/rnlib/cryptography.h"

#ifdef _WIN32
    #define SODIUM_STATIC 1
    #define SODIUM_EXPORT
#endif

#include <sodium.h>

using namespace rn;

std::optional<KeyBuffer> KeyBuffer::DeriveSingleUse(uint64_t context, uint64_t nonce)
{
    std::optional<KeyBuffer> result;
    int error = crypto_kdf_derive_from_key(
            result->data(), result->size(), nonce, (char *)context, *this);

    return !error ? result : std::nullopt;
}

std::optional<KeyPair> KeyPair::Ephemeral()
{
    std::optional<KeyPair> result;
    int error = crypto_kx_keypair(result->pub, result->sec);

    return !error ? result : std::nullopt;
}

std::optional<SessionKeys> SessionKeys::Client(
        const KeyPair &client, const KeyBuffer &server_pub)
{
    std::optional<SessionKeys> result;
    int error = crypto_kx_client_session_keys(
            result->send, result->recv, client.pub, client.sec, server_pub);

    return !error ? result : std::nullopt;
}

std::optional<SessionKeys> SessionKeys::Server(
        const KeyPair &server, const KeyBuffer &client_pub)
{
    std::optional<SessionKeys> result;
    int error = crypto_kx_client_session_keys(
            result->send, result->recv, server.pub, server.sec, client_pub);

    return !error ? result : std::nullopt;
}

void NonceBuffer::operator=(uint64_t nonce)
{
    this[2] = nonce;
}
