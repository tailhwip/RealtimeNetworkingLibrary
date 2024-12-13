#include "../include/rnlib/socket.h"

#ifdef _WIN32
    #include <ws2tcpip.h>

    #define SOCKAPI_ERR_HANDLE INVALID_SOCKET
    #define SOCKAPI_ERR_RESULT SOCKET_ERROR
    #define SOCKAPI_ERR_VALUE  WSAGetLastError()

    #define SOCKAPI_CLOSE(HANDLE)      closesocket(HANDLE)
    #define SOCKAPI_NBIO(HANDLE, DATA) ioctlsocket(HANDLE, FIONBIO, &DATA)
#else
    #include <errno.h>
    #include <fcntl.h>
    #include <netinet/in.h>
    #include <sys/socket.h>

    #define SOCKAPI_ERR_HANDLE -1
    #define SOCKAPI_ERR_RESULT -1
    #define SOCKAPI_ERR_VALUE  errno

    #define SOCKAPI_CLOSE(HANDLE)      close(HANDLE)
    #define SOCKAPI_NBIO(HANDLE, DATA) fcntl(HANDLE, F_SETFL, O_NONBLOCK, DATA)
#endif

int rnSocketsInitialize()
{
#ifdef _WIN32
    WSADATA data;
    int result = WSAStartup(MAKEWORD(2, 2), &data);
    if (result == SOCKAPI_ERR_RESULT)
    {
        return SOCKAPI_ERR_VALUE;
    }
#endif

    return RN_OK;
}

int rnSocketsCleanup()
{
#ifdef _WIN32
    int result = WSACleanup();
    if (result == SOCKAPI_ERR_RESULT)
    {
        return SOCKAPI_ERR_VALUE;
    }
#endif

    return RN_OK;
}

#define RN_SOCKET_IMPL(IP, SOCKADDR)                                           \
    struct RnSocket##IP                                                        \
    {                                                                          \
        int handle;                                                            \
    };                                                                         \
                                                                               \
    int rnSocketOpen(const RnAddress##IP *host, OUT RnSocket##IP **out)        \
    {                                                                          \
        SOCKADDR addr = rnAddressToNetwork(host);                              \
                                                                               \
        int handle = socket(addr->sa_family, SOCK_DGRAM, IPPROTO_UDP);         \
        if (handle == SOCKAPI_ERR_HANDLE)                                      \
        {                                                                      \
            return SOCKAPI_ERR_VALUE;                                          \
        }                                                                      \
                                                                               \
        int bind_result = bind(handle, addr, host_bytes);                      \
        if (bind_result == SOCKAPI_ERR_RESULT)                                 \
        {                                                                      \
            return SOCKAPI_ERR_VALUE;                                          \
        }                                                                      \
                                                                               \
        int non_blocking = 1;                                                  \
        int nbio_result  = SOCKAPI_NBIO(handle, non_blocking);                 \
        if (nbio_result == SOCKAPI_ERR_RESULT)                                 \
        {                                                                      \
            return SOCKAPI_ERR_VALUE;                                          \
        }                                                                      \
                                                                               \
        *out = malloc(sizeof RnSocket##IP);                                    \
        if (*out == NULL)                                                      \
        {                                                                      \
            return RN_OOM;                                                     \
        }                                                                      \
                                                                               \
        **out = RnSocket##IP { handle };                                       \
        return RN_OK;                                                          \
    }                                                                          \
                                                                               \
    int rnSocketClose(IN RnSocket##IP *in)                                     \
    {                                                                          \
        int result = SOCKAPI_CLOSE(in->handle);                                \
        if (result == SOCKAPI_ERR_RESULT)                                      \
        {                                                                      \
            return SOCKAPI_ERR_VALUE;                                          \
        }                                                                      \
                                                                               \
        free(in);                                                              \
        return result;                                                         \
    }                                                                          \
                                                                               \
    int rnSocketSendData(                                                      \
          const RnSocket##IP *socket, const RnAddress##IP *address,            \
          const uint8_t *data, size_t data_size)                               \
    {                                                                          \
        SOCKADDR addr = rnAddressToNetwork(*address);                          \
                                                                               \
        int result = sendto(handle, data, data_size, 0, &addr, sizeof addr);   \
        if (result == SOCKAPI_ERR_RESULT)                                      \
        {                                                                      \
            return SOCKAPI_ERR_VALUE;                                          \
        }                                                                      \
                                                                               \
        return RN_OK;                                                          \
    }                                                                          \
                                                                               \
    int rnSocketReceiveData(                                                   \
          const RnSocket##IP *socket, OUT RnAddress##IP *address,              \
          OUT uint8_t *data, OUT size_t *data_size)                            \
    {                                                                          \
        SOCKADDR addr;                                                         \
                                                                               \
        int result = recvfrom(handle, data, *data_size, 0, addr, sizeof addr); \
        if (result == SOCKAPI_ERR_RESULT)                                      \
        {                                                                      \
            return SOCKAPI_ERR_VALUE;                                          \
        }                                                                      \
                                                                               \
        *address   = rnAddressFromNetwork(addr);                               \
        *data_size = result;                                                   \
        return RN_OK;                                                          \
    }

RN_SOCKET_IMPL(IPv4, sockaddr_in)
RN_SOCKET_IMPL(IPv6, sockaddr_in6)

#undef RN_SOCKET_IMPL
