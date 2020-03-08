// Copyright (C) 2020  kid Novalis <detective7th@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _KN_NET_UDP_SERVER_H_
#define _KN_NET_UDP_SERVER_H_

#pragma once

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <folly/Likely.h>

namespace kn
{
namespace net
{
namespace udp
{

class Server
{
    using ProcFunc = std::function<void(const boost::asio::ip::udp::endpoint&,
                                        const boost::asio::mutable_buffer&,
                                        const boost::system::error_code&,
                                        const std::size_t&)>;

    static constexpr size_t kRecvBufSize_ = 8 * def::MB;
public:
    explicit Server(boost::asio::io_context& ioc, const uint16_t& port, ProcFunc func)
            :socket_(ioc, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port))
            ,func_(std::move(func))

    {
        SetOpt();
        StartRecv();
    }

    virtual ~Server()
    {
        if (nullptr != buf_)
        {
            delete [] buf_;
        }
    }

protected:
    void SetOpt()
    {
        boost::asio::socket_base::receive_buffer_size opt_send_buf(kRecvBufSize_);
        socket_.set_option(opt_send_buf);
    }

    void StartRecv()
    {
        socket_.async_receive_from(buffer_, remote_,
                                   boost::asio::socket_base::message_end_of_record,
                                   boost::bind(&Server::HandleRecv, this,
                                               boost::asio::placeholders::error,
                                               boost::asio::placeholders::bytes_transferred));
    }

    void HandleRecv(const boost::system::error_code& ec, std::size_t size_read)
    {
        if (LIKELY(!ec))
        {
            func_(remote_, buffer_, ec, size_read);
            StartRecv();
        }
        else
        {
            LOG(ERROR) << "RecvError|" << ec << "|" << ec.message();
        }
    }

protected:
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint remote_;
    char* buf_ = new char[kRecvBufSize_];
    boost::asio::mutable_buffer buffer_{buf_, kRecvBufSize_};
    ProcFunc func_;
}; // Srv

} // udp
} // net
} // kn

#endif // _KN_NET_UDP_SERVER_H_
