#include "../include/rnlib/socket.h"

#include <type_traits>

#ifdef _WIN32
    #include <ws2tcpip.h>

// clang-format off
    typedef int socklen_t;
// clang-format on
#else
    #include <errno.h>
    #include <fcntl.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
#endif

using namespace rn;

template <typename T, typename sockaddr_t> result_t Socket<T, sockaddr_t>::Open()
{
    int family = std::is_same<T, IPv4>::value ? AF_INET : AF_INET6;
    int handle = socket(family, SOCK_DGRAM, IPPROTO_UDP);
#ifdef _WIN32
    if (handle == INVALID_SOCKET)
        return WSAGetLastError();
#else
    if (handle == -1)
        return errno;
#endif

    this->handle = handle;
    return RN_RESULT_OK;
}

template <typename T, typename sockaddr_t>
result_t Socket<T, sockaddr_t>::Bind(const T &bind_addr)
{
    sockaddr_t sock_addr = bind_addr.ToSockAddr();
    int result = bind(handle, (sockaddr *)&sock_addr, sizeof(sock_addr));
#ifdef _WIN32
    if (result == SOCKET_ERROR)
        return WSAGetLastError();
#else
    if (result == -1)
        return errno;
#endif

    return RN_RESULT_OK;
}

template <typename T, typename sockaddr_t>
result_t Socket<T, sockaddr_t>::SetNonBlocking()
{
#ifdef _WIN32
    DWORD non_blocking = 1;
    int result = ioctlsocket(handle, FIONBIO, &non_blocking);
    if (result == SOCKET_ERROR)
        return WSAGetLastError();
#else
    int non_blocking = 1;
    int result = fcntl(handle, F_SETFL, O_NONBLOCK, non_blocking);
    if (result == -1)
        return errno;
#endif

    return RN_RESULT_OK;
}

template <typename T, typename sockaddr_t>
result_t Socket<T, sockaddr_t>::SendData(const T &to_addr, const PacketBuffer &buff)
{
    sockaddr_t sock_addr = to_addr.ToSockAddr();
    int result = sendto(handle, (const char *)&buff.data[0], buff.size, 0,
                        (sockaddr *)&sock_addr, sizeof(sock_addr));
#ifdef _WIN32
    if (result == SOCKET_ERROR)
        return WSAGetLastError();
#else
    if (result == -1)
        return errno;
#endif

    return RN_RESULT_OK;
}

template <typename T, typename sockaddr_t>
result_t Socket<T, sockaddr_t>::ReceiveData(_Out_ T &from_addr, _Out_ PacketBuffer &buff)
{
    sockaddr_t sock_addr;
    int sock_addr_size = sizeof(sockaddr_t);
    int result = recvfrom(handle, (char *)&buff.data[0], RN_PACKET_MAX_SIZE, 0,
                          (sockaddr *)&sock_addr, &sock_addr_size);
#ifdef _WIN32
    if (result == SOCKET_ERROR)
        return WSAGetLastError();
#else
    if (result == -1)
        return errno;
#endif

    from_addr = T(*(sockaddr_t *)&sock_addr);
    buff.size = result;
    return RN_RESULT_OK;
}

template <typename Address, typename sockaddr_t>
result_t Socket<Address, sockaddr_t>::Close()
{
#ifdef _WIN32
    int result = closesocket(handle);
    if (result != NO_ERROR)
        return WSAGetLastError();
#else
    int result = close(handle);
    if (result == -1)
        return errno;
#endif

    return RN_RESULT_OK;
}

template struct rn::Socket<IPv4>;
template struct rn::Socket<IPv6>;
