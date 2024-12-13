#include "../include/rnlib/address.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef _WIN32
    #include <ws2tcpip.h>
#else
    #include <netinet/in.h>
    #include <sys/socket.h>
#endif

bool rnAddressEquals(RnAddressIPv4 a, RnAddressIPv4 b)
{
    int octets = 0;
    for (int i = 0; i < 4; ++i)
    {
        octets += a.octets[i] == b.octets[i];
    }

    return octets == 4 && a.port == b.port;
}

bool rnAddressEquals(RnAddressIPv6 a, RnAddressIPv6 b)
{
    int groups = 0;
    for (int i = 0; i < 8; ++i)
    {
        groups += a.groups[i] == b.groups[i];
    }

    return groups == 8 && a.port == b.port;
}

RnAddressIPv4 rnAddressFromNetwork(sockaddr_in net)
{
    RnAddresIPv4 ipv4;

    uint8_t *net_bytes = (uint8_t *)&net.sin_addr.s_addr;
    for (int i = 0; i < 4; ++i)
    {
        ipv4.octets[i] = net_bytes[3 - i];
    }

    ipv4.port = ntohs(net.sin_port);
    return ipv4;
}

RnAddressIPv6 rnAddressFromNetwork(sockaddr_in6 net)
{
    RnAddressIPv6 ipv6;

    uint8_t *host_bytes = (uint8_t *)ipv6.groups;
    for (int i = 0; i < 16; ++i)
    {
        host_bytes[i] = net.sin6_addr.s6_addr[15 - i];
    }

    ipv6.port = ntohs(net.sin6_port);
    return ipv6;
}

sockaddr_in rnAddressToNetwork(RnAddressIPv4 ipv4)
{
    sockaddr_in net { .sin_family = AF_INET };

    uint8_t *net_bytes = (uint8_t *)net.sin_addr.s_addr;
    for (int i = 0; i < 4; ++i)
    {
        net_bytes[i] = ipv4.octets[3 - i];
    }

    net.sin_port = htons(ipv4.port);
    return net;
}

sockaddr_in6 rnAddressToNetwork(RnAddressIPv6 ipv6)
{
    sockaddr_in6 net { .sin6_family = AF_INET6 };

    uint8_t *host_bytes = (uint8_t *)ipv6.groups;
    for (int i = 0; i < 16; ++i)
    {
        net.sin6_addr.s6_addr[i] = host_bytes[15 - i];
    }

    net.sin6_port = htons(ipv6.port);
    return net;
}
