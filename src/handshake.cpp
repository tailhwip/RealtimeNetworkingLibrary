#include "../include/rnlib/handshake.h"

#include <optional>

#ifdef _WIN32
    #define SODIUM_STATIC 1
    #define SODIUM_EXPORT
#endif

#include <sodium.h>

using namespace rn;

template <typename T, typename ARGS_T, typename V>
Handshake<T, ARGS_T, V>::Handshake(ARGS_T args)
    : stage { HandshakeStage::DISCONNECTED }, args { args }, data {}
{
}

template <typename BUFFER_T, typename U, typename V>
HandshakeStatus Handshake<BUFFER_T, U, V>::ReadPacket(_In_ BUFFER_T &buffer)
{
    if (buffer.header.type != RN_PACKET_TYPE_HANDSHAKE)
    {
        return stage == HandshakeStage::CONNECTED ? HandshakeStatus::CONNECTED
                                                  : HandshakeStatus::PENDING;
    }

    switch (stage)
    {
        case HandshakeStage::CLIENT_HELLO:
        {
            auto message = ServerChallenge::Deserialize(buffer);
            if (message.has_value())
            {
                stage = HandshakeStage::SERVER_CHALLENGE;

                data = message->data;
            }

            return HandshakeStatus::PENDING;
        }

        case HandshakeStage::CLIENT_CHALLENGE:
        {
            auto message = ServerHello::Deserialize(buffer);
            if (message.has_value())
            {
                stage = HandshakeStage::SERVER_HELLO;
            }

            return HandshakeStatus::PENDING;
        }

        case HandshakeStage::CONNECTED:
        {
            return HandshakeStatus::CONNECTED;
        }

        default:
        {
            return HandshakeStatus::PENDING;
        }
    }
}

template <typename BUFFER_T, typename U, typename V>
std::optional<BUFFER_T> Handshake<BUFFER_T, U, V>::WritePacket()
{
    switch (stage)
    {

        case HandshakeStage::DISCONNECTED:
        case HandshakeStage::CLIENT_HELLO:
        {
            stage = HandshakeStage::CLIENT_HELLO;

            ClientHello message { .args = args };
            return ClientHello::Serialize(message);
        }

        case HandshakeStage::SERVER_CHALLENGE:
        case HandshakeStage::CLIENT_CHALLENGE:
        {
            stage = HandshakeStage::CLIENT_CHALLENGE;

            ClientChallenge message { .data = data };
            return ClientChallenge::Serialize(message);
        }

        case HandshakeStage::SERVER_HELLO:
        case HandshakeStage::CONNECTED:
        {
            stage = HandshakeStage::CONNECTED;

            return std::nullopt;
        }

            /* TODO send keep-alive */

            /* TODO impl salt renegotiation */
    }
}

#define HANDSHAKE_T Handshake<BUFFER_T, ARGS_T, DATA_T>

template <typename BUFFER_T, typename ARGS_T, typename DATA_T>
BUFFER_T HANDSHAKE_T::ClientHello::Serialize(const ClientHello &message)
{
    BUFFER_T buffer(RN_PACKET_TYPE_HANDSHAKE);

    auto stage = static_cast<uint8_t>(HandshakeStage::CLIENT_HELLO);
    buffer.SerializeUInt8(stage);

    buffer.SerializeT(message.args);

    buffer.SerializePadding();
    return buffer;
}

#define CLIENT_HELLO_T typename HANDSHAKE_T::ClientHello

template <typename BUFFER_T, typename ARGS_T, typename DATA_T>
std::optional<CLIENT_HELLO_T> HANDSHAKE_T::ClientHello::Deserialize(BUFFER_T &buffer)
{
    std::optional<ClientHello> message;
    if (buffer.byte_size() != RN_PACKET_BYTES_MAX)
    {
        return std::nullopt;
    }

    auto stage = static_cast<HandshakeStage>(buffer.DeserializeUInt8());
    if (stage != HandshakeStage::CLIENT_HELLO)
    {
        return std::nullopt;
    }

    message->args = buffer.DeserializeT();
    return message;
}

template <typename BUFFER_T, typename ARGS_T, typename DATA_T>
BUFFER_T HANDSHAKE_T::ServerChallenge::Serialize(const ServerChallenge &message)
{
    BUFFER_T buffer(RN_PACKET_TYPE_HANDSHAKE);

    auto stage = static_cast<uint8_t>(HandshakeStage::SERVER_CHALLENGE);
    buffer.SerializeUInt8(stage);

    buffer.SerializeT(message.data);
    return buffer;
}

#define SERVER_CHALLENGE_T typename HANDSHAKE_T::ServerChallenge

template <typename BUFFER_T, typename ARGS_T, typename DATA_T>
std::optional<SERVER_CHALLENGE_T> HANDSHAKE_T::ServerChallenge::Deserialize(
        BUFFER_T &buffer)
{
    std::optional<ServerChallenge> message;
    if (buffer.byte_size() != RN_PACKET_BYTES_MAX)
    {
        return std::nullopt;
    }

    auto stage = static_cast<HandshakeStage>(buffer.DeserializeUInt8());
    if (stage != HandshakeStage::SERVER_CHALLENGE)
    {
        return std::nullopt;
    }

    message->data = buffer.DeserializeT();
    return message;
}

template <typename BUFFER_T, typename ARGS_T, typename DATA_T>
BUFFER_T HANDSHAKE_T::ClientChallenge::Serialize(const ClientChallenge &message)
{
    BUFFER_T buffer(RN_PACKET_TYPE_HANDSHAKE);

    auto stage = static_cast<uint8_t>(HandshakeStage::CLIENT_CHALLENGE);
    buffer.SerializeUInt8(stage);

    buffer.SerializeT(message.data);

    buffer.SerializePadding();
    return buffer;
}

#define CLIENT_CHALLENGE_T typename HANDSHAKE_T::ClientChallenge

template <typename BUFFER_T, typename ARGS_T, typename DATA_T>
std::optional<CLIENT_CHALLENGE_T> HANDSHAKE_T::ClientChallenge::Deserialize(
        BUFFER_T &buffer)
{
    std::optional<ClientChallenge> message;
    if (buffer.byte_size() != RN_PACKET_BYTES_MAX)
    {
        return std::nullopt;
    }

    auto stage = static_cast<HandshakeStage>(buffer.DeserializeUInt8());
    if (stage != HandshakeStage::CLIENT_CHALLENGE)
    {
        return std::nullopt;
    }

    message->data = buffer.DeserializeT();
    return message;
}

template <typename BUFFER_T, typename ARGS_T, typename DATA_T>
BUFFER_T HANDSHAKE_T::ServerHello::Serialize(const ServerHello &message)
{
    BUFFER_T buffer(RN_PACKET_TYPE_HANDSHAKE);

    auto stage = static_cast<uint8_t>(HandshakeStage::SERVER_HELLO);
    buffer.SerializeUInt8(stage);

    return buffer;
}

#define SERVER_HELLO_T typename HANDSHAKE_T::ServerHello

template <typename BUFFER_T, typename ARGS_T, typename DATA_T>
std::optional<SERVER_HELLO_T> HANDSHAKE_T::ServerHello::Deserialize(BUFFER_T &buffer)
{
    std::optional<ServerHello> message;
    if (buffer.byte_size() != RN_PACKET_BYTES_MAX)
    {
        return std::nullopt;
    }

    auto stage = static_cast<HandshakeStage>(buffer.DeserializeUInt8());
    if (stage != HandshakeStage::SERVER_HELLO)
    {
        return std::nullopt;
    }

    return message;
}
