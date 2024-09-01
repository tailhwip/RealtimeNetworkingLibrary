#include "../include/rnlib/address.h"

#include <cstring>

#ifdef _WIN32
    #include <ws2tcpip.h>
#else
    #include <netinet/in.h>
    #include <sys/socket.h>
#endif

using namespace rn;

IPv4::IPv4(uint8_t octets[4], uint16_t port)
{
    *(uint16_t *)&this->octets[0] = htonl(*(uint16_t *)&octets[2]);
    *(uint16_t *)&this->octets[2] = htons(*(uint16_t *)&octets[0]);
    this->port = htons(port);
}

IPv4::IPv4(const sockaddr_in &addr)
{
    memcpy(octets, &addr.sin_addr.s_addr, sizeof(octets));
    port = addr.sin_port;
}

sockaddr_in IPv4::ToSockAddr() const
{
    sockaddr_in addr = { .sin_family = AF_INET };
    memcpy(&addr.sin_addr.s_addr, octets, sizeof(octets));
    addr.sin_port = port;

    return addr;
}

IPv6::IPv6(uint16_t groups[4], uint16_t port)
{
    for (int i = 0; i < 8; ++i)
    {
        this->groups[i] = htons(groups[7 - i]);
    }

    this->port = htons(port);
}

IPv6::IPv6(const sockaddr_in6 &addr)
{
    memcpy(groups, &addr.sin6_addr.u.Word, sizeof(groups));
    port = addr.sin6_port;
}

sockaddr_in6 IPv6::ToSockAddr() const
{
    struct sockaddr_in6 addr = { .sin6_family = AF_INET6 };
    memcpy(&addr.sin6_addr.u.Word, groups, sizeof(groups));
    addr.sin6_port = port;

    return addr;
}
