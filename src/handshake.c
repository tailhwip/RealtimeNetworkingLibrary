#include "../include/rnlib/handshake.h"

struct RnHandshakeSecure
{
    enum RnHandshakeStep step;
    RnKeyBuffer pubkey;
    uint64_t context;
};

struct RnHandshakeInsecure
{
    enum RnHandshakeStep step;
    uint64_t salt;
};

RnHandshakeSecure rnHandshakeCreateSecure(const RnKeyBuffer *pubkey)
{
    return RnHandshakeSecure {
        .step    = RN_HANDSHAKE_DISCONNECTED,
        .pubkey  = *pubkey,
        .context = 0,
    };
}

RnHandshakeSecure rnHandshakeCreateSecure(uint64_t salt)
{
    return RnHandshakeSecure {
        .step = RN_HANDSHAKE_DISCONNECTED,
        .salt = salt,
    };
}

bool rnHandshakeReadPacketClient(RnHandshakeSecure *handshake, const RnPacketBufferSecure *buffer)
{
    if (buffer.header.type != RN_HANDSHAKE_PACKET_TYPE)
    {
        return handshake->step == RN_HANDSHAKE_CONNECTED;
    }

    uint8_t type;
    int type_result = rnPacketBufferDeserializeUInt8(b, &type);
    if (type_result != RN_OK)
    {
        return false;
    }

    switch (type)
    {
        case RN_HANDSHAKE_SERVER_CHALLENGE:
        {
            if (handshake->step == RN_HANDSHAKE_CLIENT_CONNECT)
            {
                uint64_t context;
                RnPacketBufferCursor cursor = rnPacketBufferCursorInitRead(meta);
                int result = rnPacketBufferDeserializeUInt64(buffer, cursor, &context);
                if (result == RN_OK)
                {
                    handshake->context = context;

                    handshake->step = RN_HANDSHAKE_SERVER_CHALLENGE;
                }
            }

            break;
        }

        case RN_HANDSHAKE_SERVER_CONNECTED:
        {
            if (handshake->step == RN_HANDSHAKE_CLIENT_RESPONSE)
            {
                handshake->step = RN_HANDSHAKE_SERVER_CONNECTED;
            }

            break;
        }
    }

    return false;
}

bool rnHandshakeReadPacketClient(
      RnHandshakeInsecure *handshake, const RnPacketBufferInsecure *buffer)
{
    if (buffer.header.type != RN_HANDSHAKE_PACKET_TYPE)
    {
        return handshake->step == RN_HANDSHAKE_CONNECTED;
    }

    uint8_t type;
    int type_result = rnPacketBufferDeserializeUInt8(b, &type);
    if (type_result != RN_OK)
    {
        return false;
    }

    switch (type)
    {
        case RN_HANDSHAKE_SERVER_CHALLENGE:
        {
            if (handshake->step == RN_HANDSHAKE_CLIENT_CONNECT)
            {
                uint64_t salt;
                RnPacketBufferCursor cursor = rnPacketBufferCursorInitRead(meta);
                int result = rnPacketBufferDeserializeUInt64(buffer, cursor, &salt);
                if (result == RN_OK)
                {
                    handshake->salt ^= salt;

                    handshake->step = RN_HANDSHAKE_SERVER_CHALLENGE;
                }
            }

            break;
        }

        case RN_HANDSHAKE_SERVER_CONNECTED:
        {
            if (handshake->step == RN_HANDSHAKE_CLIENT_RESPONSE)
            {
                handshake->step = RN_HANDSHAKE_SERVER_CONNECTED;
            }

            break;
        }
    }

    return false;
}

bool rnHandshakeWritePacketClient(RnHandshakeSecure *handshake, OUT RnPacketBufferSecure *buffer)
{
    switch (handshake->step)
    {
        case RN_HANDSHAKE_DISCONNECTED:
        case RN_HANDSHAKE_CLIENT_CONNECT:
        {
            *buffer                     = rnPacketBufferCreateSecure(RN_HANDSHAKE_PACKET_TYPE);
            RnPacketBufferCursor cursor = rnPacketBufferCursorInitWrite();

            rnPacketBufferSerializeUInt8(buffer, cursor, RN_HANDSHAKE_CLIENT_CONNECT);
            rnPacketBufferSerializeKeyBuffer(buffer, cursor, handshake->pubkey);
            rnPacketBufferSerializePadding(buffer, cursor);

            handshake->step = RN_HANDSHAKE_CLIENT_CONNECT;
            return true;
        }

        case RN_HANDSHAKE_SERVER_CHALLENGE:
        case RN_HANDSHAKE_CLIENT_RESPONSE:
        {
            *buffer                     = rnPacketBufferCreateSecure(RN_HANDSHAKE_PACKET_TYPE);
            RnPacketBufferCursor cursor = rnPacketBufferCursorInitWrite();

            rnPacketBufferSerializeUInt8(buffer, cursor, RN_HANDSHAKE_CLIENT_RESPONSE);
            rnPacketBufferSerializeUInt64(buffer, cursor, handshake->context);
            rnPacketBufferSerializePadding(buffer, cursor);

            handshake->step = RN_HANDSHAKE_CLIENT_RESPONSE;
            return true;
        }
    }

    return false;
}

bool rnHandshakeWritePacketClient(
      RnHandshakeInsecure *handshake, OUT RnPacketBufferInsecure *buffer)
{
    switch (handshake->step)
    {
        case RN_HANDSHAKE_DISCONNECTED:
        case RN_HANDSHAKE_CLIENT_CONNECT:
        {
            *buffer                     = rnPacketBufferCreateInsecure(RN_HANDSHAKE_PACKET_TYPE);
            RnPacketBufferCursor cursor = rnPacketBufferCursorInitWrite();

            rnPacketBufferSerializeUInt8(buffer, cursor, RN_HANDSHAKE_CLIENT_CONNECT);
            rnPacketBufferSerializeUInt64(buffer, cursor, handshake->salt);
            rnPacketBufferSerializePadding(buffer, cursor);

            handshake->step = RN_HANDSHAKE_CLIENT_CONNECT;
            return true;
        }

        case RN_HANDSHAKE_SERVER_CHALLENGE:
        case RN_HANDSHAKE_CLIENT_RESPONSE:
        {
            *buffer                     = rnPacketBufferCreateInsecure(RN_HANDSHAKE_PACKET_TYPE);
            RnPacketBufferCursor cursor = rnPacketBufferCursorInitWrite();

            rnPacketBufferSerializeUInt8(buffer, cursor, RN_HANDSHAKE_CLIENT_RESPONSE);
            rnPacketBufferSerializeUInt64(buffer, cursor, handshake->salt);
            rnPacketBufferSerializePadding(buffer, cursor);

            handshake->step = RN_HANDSHAKE_CLIENT_RESPONSE;
            return true;
        }
    }

    return false;
}

bool rnHandshakeReadPacketServer(RnHandshakeSecure *handshake, RnPacketBuffer *buffer)
{
    if (buffer.header.type != RN_HANDSHAKE_PACKET_TYPE)
    {
        return step == RN_HANDSHAKE_SERVER_CONNECTED;
    }

    if (buffer.meta.body_size != 1232)
    {
        return false;
    }

    uint8_t type;
    int type_result = rnPacketBufferDeserializeUInt8(b, &type);
    if (type_result != RN_OK)
    {
        return false;
    }

    switch (type)
    {
        case RN_HANDSHAKE_CLIENT_CONNECT:
        {
            switch (handshake->step)
            {
                case RN_HANDSHAKE_DISCONNECTED:
                {
                    RnKeyBuffer key;
                    RnPacketBufferCursor cursor = rnPacketBufferCursorInitRead(meta);
                    int result = rnPacketBufferDeserializeKeyBuffer(buffer, cursor, &key);
                    if (result == RN_OK)
                    {
                        handshake->pubkey = key;

                        stage = RN_HANDSHAKE_CLIENT_CONNECT;
                    }

                    break;
                }

                case RN_HANDSHAKE_SERVER_CHALLENGE:
                {
                    stage = RN_HANDSHAKE_CLIENT_CONNECT;
                    break;
                }
            }

            break;
        }

        case RN_HANDSHAKE_CLIENT_RESPONSE:
        {
            switch (handshake->step)
            {
                case RN_HANDSHAKE_CLIENT_RESPONSE:
                {
                    uint64_t context;
                    RnPacketBufferCursor cursor = rnPacketBufferCursorInitRead(meta);
                    int result = rnPacketBufferDeserializeUInt64(buffer, &cursor, &context);
                    if (result == RN_OK && context == handshake->context)
                    {
                        stage = RN_HANDSHAKE_CLIENT_RESPONSE;
                    }

                    break;
                }

                case RN_HANDSHAKE_SERVER_CONNECTED:
                {
                    stage = RN_HANDSHAKE_CLIENT_RESPONSE;
                    break;
                }
            }

            break;
        }
    }

    return false;
}

bool rnHandshakeSendPacketServer(RnHandshakeSecure *handshake, OUT RnPacketBufferSecure *buffer)
{
    switch (handshake->step)
    {
        case RN_HANDSHAKE_STAGE_CLIENT_HELLO:
        {
            *buffer                     = rnPacketBufferCreateSecure(RN_HANDSHAKE_PACKET_TYPE);
            RnPacketBufferCursor cursor = rnPacketBufferCursorInitWrite();

            rnPacketBufferSerializeUInt8(buffer, cursor, RN_HANDSHAKE_SERVER_CHALLENGE);
            rnPacketBufferSerializeUInt64(buffer, cursor, handshake->context);

            handshake->stage = RN_HANDSHAKE_STAGE_SERVER_CHALLENGE;
            return true;
        }

        case RN_HANDSHAKE_STAGE_CLIENT_CHALLENGE:
        {
            *buffer                     = rnPacketBufferCreateSecure(RN_HANDSHAKE_PACKET_TYPE);
            RnPacketBufferCursor cursor = rnPacketBufferCursorInitWrite();

            rnPacketBufferSerializeUInt8(buffer, cursor, RN_HANDSHAKE_SERVER_CONNECTED);

            handshake->stage = RN_HANDSHAKE_STAGE_SERVER_CONNECTED;
            return true;
        }
    }

    return false;
}

bool rnHandshakeSendPacketServer(RnHandshakeInsecure *handshake, OUT RnPacketBufferInsecure *buffer)
{
    switch (handshake->step)
    {
        case RN_HANDSHAKE_STAGE_CLIENT_HELLO:
        {
            *buffer                     = rnPacketBufferCreateInsecure(RN_HANDSHAKE_PACKET_TYPE);
            RnPacketBufferCursor cursor = rnPacketBufferCursorInitWrite();

            rnPacketBufferSerializeUInt8(buffer, cursor, RN_HANDSHAKE_SERVER_CHALLENGE);
            rnPacketBufferSerializeUInt64(buffer, cursor, handshake->salt);

            handshake->stage = RN_HANDSHAKE_STAGE_SERVER_CHALLENGE;
            return true;
        }

        case RN_HANDSHAKE_STAGE_CLIENT_CHALLENGE:
        {
            *buffer                     = rnPacketBufferCreateInsecure(RN_HANDSHAKE_PACKET_TYPE);
            RnPacketBufferCursor cursor = rnPacketBufferCursorInitWrite();

            rnPacketBufferSerializeUInt8(buffer, cursor, RN_HANDSHAKE_SERVER_CONNECTED);

            handshake->stage = RN_HANDSHAKE_STAGE_SERVER_CONNECTED;
            return true;
        }
    }

    return false;
}
