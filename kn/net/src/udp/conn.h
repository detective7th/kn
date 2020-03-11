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

#ifndef _KN_NET_UDP_CONN_H_
#define _KN_NET_UDP_CONN_H_

#pragma once
#include <folly/Likely.h>

namespace kn
{
namespace net
{
namespace udp
{

class Conn
{
public:
    Conn(boost::asio::io_context& ioc, const std::string& ip, uint16_t port, boost::system::error_code& ec)
            :socket_(ioc)
            ,ep_(boost::asio::ip::udp::endpoint(boost::asio::ip::make_address(ip.c_str(),ec), port))
    {
        socket_.open(boost::asio::ip::udp::v4());
        if ("0.0.0.0" == ip || "127.0.0.1" == ip)
        {
            boost::asio::socket_base::do_not_route opt_no_route(true);
            socket_.set_option(opt_no_route);
        }
        boost::asio::socket_base::send_buffer_size opt_send_buf(1 * def::MB);
        socket_.set_option(opt_send_buf);
    }

    Conn(Conn&& other)
        :socket_(std::move(other.socket_))
        ,ep_(std::move(other.ep_)) {}

    virtual ~Conn()
    {
        //if (nullptr != socket_)
        //{
        //    delete socket_;
        //    socket_ = nullptr;
        //}
    }

    size_t Send(const std::string& data, boost::system::error_code& ec)
    {
        size_t sended = socket_.send_to(boost::asio::buffer(data.c_str(), data.size()),
                                        ep_, boost::asio::socket_base::message_end_of_record, ec);
        if (UNLIKELY(0 != ec.value()))
        {
            G3LOG(ERROR) << "UDP|Send|" << ep_ << '|' << sended << "|" << data.size() << "|" << ec << "|" << ec.message();
        }
        else
        {
            // G3LOG(DEBUG) << "UDP|Send|" << ep_ << "|" << sended;
        }
        return sended;
    }

    size_t Send(const char* data, const size_t& send_size, boost::system::error_code& ec)
    {
        size_t sended = socket_.send_to(boost::asio::buffer(data, send_size),
                                        ep_, boost::asio::socket_base::message_end_of_record, ec);
        if (UNLIKELY(0 != ec.value()))
        {
            G3LOG(ERROR) << "UDP|Send|" << ep_ << '|' << sended << "|" << send_size << "|" << ec << "|" << ec.message();
        }
        else
        {
            // G3LOG(DEBUG) << "UDP|Send|" << ep_ << "|" << sended;
        }
        return sended;
    }

protected:
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint ep_;
};

} // udp
} // net
} // kn

#endif // _KN_NET_UDP_CONN_H_
