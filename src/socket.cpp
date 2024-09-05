#include "../include/rnlib/socket.h"

#include <expected>
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

template <typename T>
Socket<T>::Socket(int handle) : handle { handle }
{
}

template <typename T>
Socket<T>::~Socket()
{
#ifdef _WIN32
    closesocket(handle);
#else
    close(handle);
#endif
}

template <typename T>
std::optional<int> Socket<T>::SendData(
        const T &to_addr, const void *buffer, size_t buffer_size)
{
    using sockaddr_t = T::sockaddr_t;

    const char *data = (const char *)buffer;
    sockaddr_t dst = to_addr.ToSockAddr();
    int result = sendto(handle, data, buffer_size, 0, (sockaddr *)&dst, sizeof(dst));

#ifdef _WIN32
    if (result == SOCKET_ERROR)
    {
        return WSAGetLastError();
    }
#else
    if (result == -1)
    {
        return errno;
    }
#endif

    return std::nullopt;
}

template <typename T>
std::optional<int> Socket<T>::RecvData(
        _Out_ T &from_addr, _Out_ void *buffer, _Inout_ size_t &buffer_size)
{
    using sockaddr_t = T::sockaddr_t;

    char *data = (char *)buffer;
    sockaddr_t src;
    int src_size = sizeof(sockaddr_t);
    int result = recvfrom(handle, data, buffer_size, 0, (sockaddr *)&src, &src_size);

#ifdef _WIN32
    if (result == SOCKET_ERROR)
    {
        return WSAGetLastError();
    }
#else
    if (result == -1)
    {
        return errno;
    }
#endif

    from_addr = T(*(sockaddr_t *)&src);
    buffer_size = result;

    return std::nullopt;
}

template <typename T>
SocketFactory<T>::SocketFactory()
{
#ifdef _WIN32
    WSADATA data;
    WSAStartup(MAKEWORD(2, 2), &data);
#endif
}

template <typename T>
SocketFactory<T>::~SocketFactory()
{
#ifdef _WIN32
    WSACleanup();
#endif
}

template <typename T>
std::expected<Socket<T>, int> SocketFactory<T>::CreateSocket(
        const T &bind_addr, bool blocking)
{
    using sockaddr_t = T::sockaddr_t;

    int family = std::is_same<T, IPv4>::value ? AF_INET : AF_INET6;
    int handle = socket(family, SOCK_DGRAM, IPPROTO_UDP);
#ifdef _WIN32
    if (handle == INVALID_SOCKET)
    {
        return std::unexpected(WSAGetLastError());
    }
#else
    if (handle == -1)
    {
        return std::unexpected(errno);
    }
#endif

    sockaddr_t host = bind_addr.ToSockAddr();
    int bind_result = bind(handle, (sockaddr *)&host, sizeof(host));
#ifdef _WIN32
    if (bind_result == SOCKET_ERROR)
    {
        return std::unexpected(WSAGetLastError());
    }
#else
    if (bind_result == -1)
    {
        return std::unexpected(errno);
    }
#endif

    if (!blocking)
    {
#ifdef _WIN32
        DWORD non_blocking = 1;
        int nbio_result = ioctlsocket(handle, FIONBIO, &non_blocking);
        if (nbio_result == SOCKET_ERROR)
        {
            return std::unexpected(WSAGetLastError());
        }
#else
        int non_blocking = 1;
        int nbio_result = fcntl(handle, F_SETFL, O_NONBLOCK, non_blocking);
        if (nbio_result == -1)
        {
            return std::unexpected(errno);
        }
#endif
    }

    return Socket<T>(handle);
}

template class rn::Socket<IPv4>;
template class rn::Socket<IPv6>;

template class rn::SocketFactory<IPv4>;
template class rn::SocketFactory<IPv6>;
