#pragma once

#include <array>
#include <cstdint>

struct sockaddr_in;
struct sockaddr_in6;

namespace rn
{

struct IPv4
{
    using Octets = std::array<uint8_t, 4>;
    using sockaddr_t = sockaddr_in;

public:
    IPv4(Octets octets, uint16_t port);
    IPv4(const sockaddr_t &address);

    sockaddr_t ToSockAddr() const;

private:
    Octets octets;
    uint16_t port;
};

struct IPv6
{
    using Groups = std::array<uint16_t, 8>;
    using sockaddr_t = sockaddr_in6;

public:
    IPv6(Groups groups, uint16_t port);
    IPv6(const sockaddr_t &address);

    sockaddr_t ToSockAddr() const;

private:
    Groups groups;
    uint16_t port;
};

} // namespace rn
