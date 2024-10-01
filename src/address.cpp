#include "../include/rnlib/address.h"

#include <algorithm>
#include <cstring>

#ifdef _WIN32
    #include <ws2tcpip.h>
#else
    #include <netinet/in.h>
    #include <sys/socket.h>
#endif

using namespace rn;

template <typename T, typename GROUP_T, size_t V>
Address<T, GROUP_T, V>::Address(Groups groups, uint16_t port)
{
    if constexpr (std::is_same<GROUP_T, uint16_t>::value)
    {
        std::transform(groups.begin(), groups.end(), groups.begin(), htons);
    }

    std::reverse(groups.begin(), groups.end());
    this->groups = groups;
    this->port = htons(port);
}

template <typename T, typename U, size_t V>
bool Address<T, U, V>::operator==(const Address &other) const
{
    return this->groups == other.groups && this->port == other.port;
}

#define SPECIALIZE_ADDRESS_T(TYPENAME, ADDR_FAMILY, FAMILY_ATTR, GROUPS_ATTR, PORT_ATTR) \
                                                                                         \
    template <>                                                                          \
    const int TYPENAME::address_family = ADDR_FAMILY;                                    \
                                                                                         \
    template <>                                                                          \
    TYPENAME::Address(const TYPENAME::sockaddr_t &addr)                                  \
    {                                                                                    \
        memcpy(groups.data(), &addr.GROUPS_ATTR, sizeof(groups));                        \
        port = addr.PORT_ATTR;                                                           \
    }                                                                                    \
                                                                                         \
    template <>                                                                          \
    TYPENAME::sockaddr_t TYPENAME::ToSockAddr() const                                    \
    {                                                                                    \
        TYPENAME::sockaddr_t addr = { .FAMILY_ATTR = address_family };                   \
        memcpy(&addr.GROUPS_ATTR, groups.data(), sizeof(groups));                        \
        addr.PORT_ATTR = port;                                                           \
                                                                                         \
        return addr;                                                                     \
    }

SPECIALIZE_ADDRESS_T(IPv4, AF_INET, sin_family, sin_addr.s_addr, sin_port);
SPECIALIZE_ADDRESS_T(IPv6, AF_INET6, sin6_family, sin6_addr.u.Word, sin6_port);
