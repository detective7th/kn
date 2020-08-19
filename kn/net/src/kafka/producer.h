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

#ifndef _KN_NET_KAFKA_PRODUCER_H_
#define _KN_NET_KAFKA_PRODUCER_H_

#pragma once
#include <folly/Likely.h>
#include <cppkafka/cppkafka.h>
#include <librdkafka/rdkafkacpp.h>

namespace kn
{
namespace net
{
namespace kafka
{

class Producer
{
public:
    Producer(boost::asio::io_context& ioc, const std::string& topic,const std::string& addrs, bool flushFlag, boost::system::error_code& ec)
            :topic_(topic),flush_flag_(flushFlag)
    {
        cppkafka::Configuration config = {
            { "metadata.broker.list", addrs }
        };

        producer_ = std::make_shared<cppkafka::Producer>(config);
    }

    virtual ~Producer()
    {
        //if (nullptr != socket_)
        //{
        //    delete socket_;
        //    socket_ = nullptr;
        //}
    }

    void Send(const std::string& data, boost::system::error_code& ec)
    {
        try{
            producer_->produce(cppkafka::MessageBuilder(topic_).payload(data));

            if (flush_flag_)
            {

                producer_->flush();
            }
        }
        catch(std::exception& e)
        {
            G3LOG(ERROR) << e.what();
        }
    }

    size_t Send(const char* data, const size_t& send_size, boost::system::error_code& ec)
    {
        try{
            producer_->produce(cppkafka::MessageBuilder(topic_).payload(cppkafka::Buffer(data, send_size)));

            if (flush_flag_)
            {
                producer_->flush();
            }
        }
        catch(std::exception& e)
        {
            G3LOG(ERROR) << e.what();
        }
    }

    void Send(const std::string& data, boost::system::error_code& ec, const std::string& topic)
    {
        try{
            if (topic.empty())
            {
                producer_->produce(cppkafka::MessageBuilder(topic_).payload(data));
            }
            else
            {
                producer_->produce(cppkafka::MessageBuilder(topic).payload(data));
            }

            if (flush_flag_)
            {
                producer_->flush();
            }
         }
        catch(std::exception& e)
        {
            G3LOG(ERROR) << e.what();
        }
    }

    size_t Send(const char* data, const size_t& send_size, boost::system::error_code& ec, const std::string& topic)
    {
        try{
            if (topic.empty())
            {
                producer_->produce(cppkafka::MessageBuilder(topic_).payload(cppkafka::Buffer(data, send_size)));
            }
            else
            {
                producer_->produce(cppkafka::MessageBuilder(topic).payload(cppkafka::Buffer(data, send_size)));
            }

            if (flush_flag_)
            {
                producer_->flush();
            }
        }
        catch(std::exception& e)
        {
            G3LOG(ERROR) << e.what();
        }
    }

protected:
    bool flush_flag_;
    std::string topic_;
    std::shared_ptr<cppkafka::Producer> producer_;
};

} // udp
} // net
} // kn

#endif // _KN_NET_UDP_CONN_H_
