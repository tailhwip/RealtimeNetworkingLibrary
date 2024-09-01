#pragma once

#include <array>
#include <cstdint>

#define RN_PACKET_MAX_SIZE 508

namespace rn
{

struct PacketBuffer
{
    std::array<uint8_t, RN_PACKET_MAX_SIZE> data;
    size_t size;
};

} // namespace rn
