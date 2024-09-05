#pragma once

#include "address.h"

#include <expected>
#include <optional>

namespace rn
{

template <typename IPvT>
class Socket
{
    template <typename>
    friend class SocketFactory;

public:
    ~Socket();

    std::optional<int> SendData(
        const IPvT &to_address,
        const void *buffer,
        size_t buffer_size);

    std::optional<int> RecvData(
        _Out_ IPvT &from_address,
        _Out_ void *buffer,
        _Inout_ size_t &buffer_size);

private:
    Socket(int handle);

    int handle;
};

template <typename IPvT>
class SocketFactory
{
public:
    SocketFactory();
    ~SocketFactory();

    std::expected<Socket<IPvT>, int> CreateSocket(
        const IPvT &bind_address,
        bool blocking = false);
};

} // namespace rn
