#pragma once

#include <compare>
#include <cstdint>

struct sockaddr_in;
struct sockaddr_in6;

namespace rn
{

struct IPv4
{
    using sockaddr_t = sockaddr_in;

public:
    IPv4(uint8_t octets[4], uint16_t port);
    IPv4(const sockaddr_t &address);

    auto operator<=>(const IPv4 &) const = default;

    sockaddr_t ToSockAddr() const;

private:
    uint8_t octets[4];
    uint16_t port;
};

struct IPv6
{
    using sockaddr_t = sockaddr_in6;

public:
    IPv6(uint16_t groups[8], uint16_t port);
    IPv6(const sockaddr_t &address);

    auto operator<=>(const IPv6 &) const = default;

    sockaddr_t ToSockAddr() const;

private:
    uint16_t groups[8];
    uint16_t port;
};

} // namespace rn
