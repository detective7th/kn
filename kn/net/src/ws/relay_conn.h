/*
 * Copyright (C) 2019  kid Novalis <detective7th@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _KN_NET_WS_REALY_CONN_H_
#define _KN_NET_WS_REALY_CONN_H_

#pragma once
#include "../udp/conn.h"
#include "parasitism.h"

namespace kn
{
namespace net
{
namespace ws
{

class UdpReceivers
{
public:
    explicit UdpReceivers(std::vector<kn::net::udp::Conn>& receivers)
            :receivers_(receivers) {}

    void Send(const char*& data, const size_t& size, boost::system::error_code& ec)
    {
        for (auto& receiver: receivers_)
        {
            receiver.Send(data, size, ec);
        }
    }

    void Send(const std::string& data, boost::system::error_code& ec)
    {
        for (auto& receiver: receivers_)
        {
            receiver.Send(data, ec);
        }
    }

    void Send(boost::system::error_code& ec)
    {
        for (auto& receiver: receivers_)
        {
            receiver.Send(send_buf_, ec);
        }
    }

protected:
    std::vector<kn::net::udp::Conn>& receivers_;
    std::string send_buf_;
};

template<bool EnableSSL> class RelayConn : public Parasitism<EnableSSL>, UdpReceivers
{

};

template<>
class RelayConn<true> : public Parasitism<true>, public UdpReceivers
{
public:
    RelayConn(boost::asio::io_context& ioc,
              boost::asio::ssl::context& ssl,
              const kn::net::ws::Entry& entry,
              const std::string& cmd,
              const std::vector<std::string>& sub,
              Parasitifer<true>* parasitifer,
              std::vector<kn::net::udp::Conn>& receivers)
            :Parasitism<true>(ioc, ssl, entry, cmd, sub, parasitifer)
            ,UdpReceivers(receivers){}
}; // RelayConn

template<>
class RelayConn<false> : public Parasitism<false>, public UdpReceivers
{
public:
    RelayConn(boost::asio::io_context& ioc,
              const kn::net::ws::Entry& entry,
              const std::string& cmd,
              const std::vector<std::string>& sub,
              Parasitifer<false>* parasitifer,
              std::vector<kn::net::udp::Conn>& receivers)
            :Parasitism<false>(ioc, entry, cmd, sub, parasitifer)
            ,UdpReceivers(receivers){}
}; // RelayConn

} // ws
} // net
} // kn

#endif // _KN_NET_WS_REALY_CONN_H_
