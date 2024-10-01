#pragma once

#include "cryptography.h"
#include "packet.h"

#include <optional>

namespace rn
{

enum class HandshakeStatus
{
    CONNECTED,
    PENDING,
};

enum class HandshakeStage
{
    DISCONNECTED,
    CLIENT_HELLO,
    SERVER_CHALLENGE,
    CLIENT_CHALLENGE,
    SERVER_HELLO,
    CONNECTED,
};

template <typename BUFFER_T, typename ARGS_T, typename DATA_T>
class Handshake
{
public:
    HandshakeStage stage;
    ARGS_T args;
    DATA_T data;

    Handshake(ARGS_T args);

    HandshakeStatus ReadPacket(_In_ BUFFER_T &buffer);
    std::optional<BUFFER_T> WritePacket();

    struct ClientHello
    {
        ARGS_T args;

        static BUFFER_T Serialize(const ClientHello &);
        static std::optional<ClientHello> Deserialize(BUFFER_T &);
    };

    struct ServerChallenge
    {
        DATA_T data;

        static BUFFER_T Serialize(const ServerChallenge &);
        static std::optional<ServerChallenge> Deserialize(BUFFER_T &);
    };

    struct ClientChallenge
    {
        DATA_T data;

        static BUFFER_T Serialize(const ClientChallenge &);
        static std::optional<ClientChallenge> Deserialize(BUFFER_T &);
    };

    struct ServerHello
    {
        static BUFFER_T Serialize(const ServerHello &);
        static std::optional<ServerHello> Deserialize(BUFFER_T &);
    };
};

using SecureHandshake = Handshake<SecurePacketBuffer, KeyBuffer, uint64_t>;
using InsecureHandshake = Handshake<InsecurePacketBuffer, uint64_t, uint64_t>;

} // namespace rn
