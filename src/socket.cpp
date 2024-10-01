#include "../include/rnlib/socket.h"

#include <string>

#ifdef _WIN32
    #include <ws2tcpip.h>

    #define ERR_SOCKET_HANDLE INVALID_SOCKET
    #define ERR_SOCKET_RESULT SOCKET_ERROR
    #define ERR_VALUE WSAGetLastError()
#else
    #include <errno.h>
    #include <fcntl.h>
    #include <netinet/in.h>
    #include <sys/socket.h>

    #define ERR_SOCKET_HANDLE -1
    #define ERR_SOCKET_RESULT -1
    #define ERR_VALUE errno
#endif

#define THROW_EXCEPTION(message)                                                         \
    std::string s = std::string(message) + " (" + std::to_string(ERR_VALUE) + ")";       \
    throw SocketException(ERR_VALUE, std::move(s), __FILE__, __LINE__);

using namespace rn;

template <typename ADDRESS_T>
Socket<ADDRESS_T>::Socket(const ADDRESS_T &bind_addr)
{
    // initialize handle
    int handle = socket(ADDRESS_T::address_family, SOCK_DGRAM, IPPROTO_UDP);
    if (handle == ERR_SOCKET_HANDLE)
    {
        THROW_EXCEPTION("Failed to acquire socket handle!");
    }

    // bind to address
    auto addr = bind_addr.ToSockAddr();
    int bind_result = bind(handle, (sockaddr *)&addr, sizeof(addr));
    if (bind_result == ERR_SOCKET_RESULT)
    {
        THROW_EXCEPTION("Failed to bind socket to address!");
    }

    // set non-blocking
#ifdef _WIN32
    DWORD non_blocking = 1;
    int nbio_result = ioctlsocket(handle, FIONBIO, &non_blocking);
#else
    int non_blocking = 1;
    int nbio_result = fcntl(handle, F_SETFL, O_NONBLOCK, non_blocking);
#endif
    if (nbio_result == ERR_SOCKET_RESULT)
    {
        THROW_EXCEPTION("Failed to set socket non-blocking!");
    }

    this->handle = handle;
}

template <typename T>
Socket<T>::Socket(Socket &&other)
{
    this->handle = other.handle;
    other.handle = 0;
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

template <typename ADDRESS_T>
void Socket<ADDRESS_T>::SendData(
        const ADDRESS_T &to_addr, const char *data, size_t data_size)
{
    auto addr = to_addr.ToSockAddr();
    int addr_size = sizeof(addr);
    int result = sendto(handle, data, data_size, 0, (sockaddr *)&addr, addr_size);
    if (result == ERR_SOCKET_RESULT)
    {
        THROW_EXCEPTION("Failed to send data from socket!");
    }
}

template <typename ADDRESS_T>
void Socket<ADDRESS_T>::ReceiveData(
        _Out_ ADDRESS_T &from_addr, _Out_ char *buff, _Inout_ size_t &buff_size)
{
    typename ADDRESS_T::sockaddr_t addr;
    int addr_size = sizeof(addr);
    int result = recvfrom(handle, buff, buff_size, 0, (sockaddr *)&addr, &addr_size);
    if (result == ERR_SOCKET_RESULT)
    {
        THROW_EXCEPTION("Failed to receive data from socket!");
    }

    from_addr = ADDRESS_T(addr);
    buff_size = result;
}

SocketsInitializer::SocketsInitializer()
{
#ifdef _WIN32
    WSADATA data;
    int result = WSAStartup(MAKEWORD(2, 2), &data);
    if (result != NO_ERROR)
    {
        THROW_EXCEPTION("Failed to initialize WSA!");
    }
#endif
}

SocketsInitializer::~SocketsInitializer()
{
#ifdef _WIN32
    WSACleanup();
#endif
}

SocketException::SocketException(
        int code, std::string message, std::string file, int line)
    : code { code }, message { message }, location { file + ":" + std::to_string(line) }
{
}
