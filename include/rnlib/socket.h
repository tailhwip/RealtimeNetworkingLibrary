#pragma once

#include "address.h"

#include <string>

namespace rn
{

template <typename ADDRESS_T>
class Socket
{
private:
    int handle;

public:
    Socket(const ADDRESS_T &bind_address);
    Socket(const Socket &) = delete;
    Socket(Socket &&);
    ~Socket();

    void SendData(
        const ADDRESS_T &to_address,
        const char *data,
        size_t data_bytes);

    void ReceiveData(
        _Out_ ADDRESS_T &from_address,
        _Out_ char *buffer,
        _Inout_ size_t &buffer_bytes);
};

class SocketsInitializer
{
public:
    SocketsInitializer();
    SocketsInitializer(const SocketsInitializer &) = delete;
    SocketsInitializer(SocketsInitializer &&) = delete;
    ~SocketsInitializer();
};

class SocketException
{
private:
    int code;
    std::string message;
    std::string location;

public:
    SocketException(int code, std::string message, std::string file, int line);

    inline int error() const noexcept { return code; }
    inline const std::string &what() const noexcept { return message; }
    inline const std::string &where() const noexcept { return location; }
};

} // namespace rn
