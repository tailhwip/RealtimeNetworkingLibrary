#include "../include/rnlib/sockets.h"

#ifdef _WIN32
    #include <ws2tcpip.h>
#else
    #include <netinet/in.h>
    #include <sys/socket.h>
#endif

using namespace rn;

result_t Sockets::Setup()
{
#ifdef _WIN32
    WSADATA data;
    int result = WSAStartup(MAKEWORD(2, 2), &data);
    if (result != NO_ERROR)
        return WSAGetLastError();
#endif

    return RN_RESULT_OK;
}

result_t Sockets::Cleanup()
{
#ifdef _WIN32
    int result = WSACleanup();
    if (result != NO_ERROR)
        return WSAGetLastError();
#endif

    return RN_RESULT_OK;
}
