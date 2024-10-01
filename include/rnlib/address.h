#pragma once

#include <array>
#include <cstdint>

struct sockaddr_in;
struct sockaddr_in6;

namespace rn
{

template <typename SOCKADDR_T, typename GROUP_T, size_t GROUPS_SIZE>
class Address
{
public:
    using Groups = std::array<GROUP_T, GROUPS_SIZE>;
    using sockaddr_t = SOCKADDR_T;

    static const int address_family;

private:
    Groups groups;
    uint16_t port;

public:
    Address(Groups groups, uint16_t port);
    Address(const sockaddr_t &address);

    bool operator==(const Address &other) const;

    sockaddr_t ToSockAddr() const;
};

using IPv4 = Address<sockaddr_in, uint8_t, 4>;
using IPv6 = Address<sockaddr_in6, uint16_t, 8>;

} // namespace rn
