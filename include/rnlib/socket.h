#pragma once

#include "address.h"
#include "packet.h"
#include "result.h"

namespace rn
{

template <typename AddressT, typename sockaddr_t = AddressT::sockaddr_t> struct Socket
{
public:
    result_t Open();

    result_t Bind(const AddressT &bind_address);

    result_t SetNonBlocking();

    result_t SendData(const AddressT &to_address, const PacketBuffer &buffer);

    result_t ReceiveData(_Out_ AddressT &from_address, _Out_ PacketBuffer &buffer);

    result_t Close();

private:
    int handle;
};

} // namespace rn
