#pragma once

#include <array>
#include <cstdint>
#include <optional>

namespace rn
{

struct KeyBuffer : std::array<uint8_t, 32>
{
    inline operator uint8_t *() { return *this; }
    inline operator const uint8_t *() const { return *this; }

    /**
     * Derives a single-use key from this buffer using the given context and
     * nonce. Neither the context nor the nonce need to be private, but care
     * should be taken that keys generated by this function are used no more
     * than exactly one time.
     */
    std::optional<KeyBuffer> DeriveSingleUse(uint64_t context, uint64_t nonce);
};

struct KeyPair
{
    KeyBuffer pub;
    KeyBuffer sec;

    /**
     * Generates an ephemeral public/secret key pair meant to be destroyed at
     * the end of the session.
     */
    static std::optional<KeyPair> Ephemeral();
};

struct SessionKeys
{
    KeyBuffer send;
    KeyBuffer recv;

    /**
     * Derives a shared session key pair for use by the client from the provided
     * client key pair and server public key.
     */
    static std::optional<SessionKeys> Client(
        const KeyPair &client_keys,
        const KeyBuffer &server_public_key);

    /**
     * Derives a shared session key pair for use by the server from the provided
     * server key pair and client public key.
     */
    static std::optional<SessionKeys> Server(
        const KeyPair &server_keys,
        const KeyBuffer &client_public_key);
};

struct NonceBuffer : std::array<uint64_t, 3>
{
    inline operator uint8_t *() { return *this; }
    inline operator const uint8_t *() const { return *this; }

    void operator=(uint64_t nonce);
};

} // namespace rn