// Copyright (C) 2019  kid Novalis <detective7th@gmail.com>
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

#ifndef _KN_NET_KAFKA_CONN_H_
#define _KN_NET_KAFKA_CONN_H_

#pragma once
//#include <g3log/g3log.hpp>
#include <folly/Likely.h>
#include <kn/log/logger.h>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <cppkafka/cppkafka.h>
#include <librdkafka/rdkafkacpp.h>
#include <iostream>
#include "base_config.h"

namespace kn
{
namespace net
{
namespace kafka
{

using KafkaConsumer = cppkafka::Consumer;
using Entry = boost::asio::ip::basic_resolver_entry<boost::asio::ip::tcp>;

class StreamEntry
{
public:
    StreamEntry(const Entry& entry,
                const std::string& cmd,
                const BaseConfig& base_config,
                const std::vector<std::string>& subs)
            :entry_(entry)
            ,cmd_(cmd)
            ,base_config_(base_config)
            ,subs_(subs)
    {
        // buffer_.reserve(1 << 18);/**/
    }

    virtual ~StreamEntry()
    {

    }

    virtual void Init()
    {
        cppkafka::Configuration config = {
               // { "metadata.broker.list", entry_.host_name()+":"+entry_.service_name()},
                { "metadata.broker.list", base_config_.hosts_},
                { "enable.auto.commit", base_config_.auto_commit_ },
                //{ "queue.buffering.max.ms", base_config_.queue_buffering_max_ms_ },
                { "fetch.wait.max.ms", base_config_.fetch_wait_max_ms_},
                { "group.id",base_config_.group_id_}
                //{"isolation.level", "read_uncommitted"},
               // {"enable.auto.offset.store", false}
        };
        if (base_config_.auto_commit_ == true)
        {
            consumer_ = std::make_shared<KafkaConsumer>(config);
            return;
        }

        if (base_config_.offset_ == 0 || base_config_.offset_ == -2)
        {
            config.set("auto.offset.reset", "smallest");
        }
        else if(base_config_.offset_ == -1)
        {
            config.set("auto.offset.reset", "largest");
        }
        consumer_ = std::make_shared<KafkaConsumer>(config);
    }

    boost::asio::ip::basic_endpoint<boost::asio::ip::tcp> endpoint() const
    {
        return entry_.endpoint();
    }
    std::shared_ptr<KafkaConsumer> stream() const
    {
        return consumer_;
    }
    auto host_name() const
    {
        return entry_.host_name();
    }

    inline auto Read()
    {
        return stream()->poll();
    }

    void Close(boost::asio::yield_context& yield, boost::beast::error_code& ec)
    {
        G3LOG(INFO) << "Close|" << endpoint() ;
    }

    void Close(boost::beast::error_code& ec)
    {
        G3LOG(INFO) << "Close|" << endpoint();
    }

    void Close()
    {
        G3LOG(INFO) << "Close|" << endpoint();
    }

    void UnSub(boost::asio::yield_context& yield, boost::beast::error_code& ec)
    {

    }

    void Sub(boost::asio::yield_context& yield, boost::beast::error_code& ec)
    {
        if (subs_.empty())
        {
            G3LOG(ERROR) << "Sub|" << endpoint() << "|no subs";
            return;
        }

        for (const auto& sub : subs_)
        {
            G3LOG(INFO) << "Sub|" << endpoint() << "|" << sub;
        }

        if(base_config_.offset_ > 0)
        {
            //不能用subscribe，不然offset设置无效
            std::vector<cppkafka::TopicPartition> partitions;
            partitions.push_back(cppkafka::TopicPartition(base_config_.subs_[0], base_config_.partitions_, base_config_.offset_));
            consumer_->assign(partitions);
            G3LOG(INFO)<<"consumer_ partitions="<<partitions<<"|offset="<<base_config_.offset_;
        }else{
            consumer_->subscribe(base_config_.subs_);
        }

        auto tpl = consumer_->get_assignment();
        G3LOG(INFO)<<"Begin offset|"<<tpl;
    }

    void Stop()
    {
        stop_ = true;
        boost::beast::error_code ec;
        Close(ec);
    }

protected:

    std::shared_ptr<KafkaConsumer> consumer_{nullptr};
    const Entry& entry_;
    std::string cmd_;
    const BaseConfig& base_config_;
    std::vector<std::string> subs_;
    bool stop_{false};
};

enum Ctrl
{
    CtrlContinue = 0,
    CtrlReConnect = 1,
    CtrlPing = 2,
    CtrlPong = 3,
    CtrlUnSub = 4,
    CtrlReSub = 5,
};

template<bool EnableSSL> class Conn;

template<>
class Conn<true> : public StreamEntry
{
public:
    Conn(boost::asio::io_context& ioc,
         boost::asio::ssl::context& ssl,
         const kn::net::kafka::Entry& entry,
         const std::string& cmd,
         const BaseConfig& base_config,
         const std::vector<std::string>& subs)
            :StreamEntry(entry, cmd, base_config, subs)
    {


    }

    virtual ~Conn() {}

    void DoSession(boost::asio::yield_context yield)
    {
        boost::beast::error_code ec;
  START:
        Init();
        StreamEntry::Init();
        Sub(yield, ec);

        while (!stop_)
        {
            if (stop_ ) break;
            const auto& msg = Read();
            if (stop_) break;

            if (UNLIKELY(!msg)) {
                continue;
            }
            if (msg.is_eof()) {
                continue;
            }

            // Messages can contain error notifications rather than actual data
            if (UNLIKELY(bool(msg.get_error()))) {
                G3LOG(ERROR) << "Read|" << endpoint() << "|" << msg.get_error().to_string();
                continue;
            }


            {
                Ctrl ctrl = OnRead(msg, yield, ec);
                G3LOG_IF(ERROR, !!ec) << "OnRead|" << endpoint() << "|"
                                    << ec << "|" << ec.message() << "|"
                                    << (msg.get_payload().get_data());
            }
        }
        Close(yield, ec);
    }

    virtual void Init() = 0;
    virtual Ctrl OnRead(const cppkafka::Message& msg,
                        boost::asio::yield_context& yield, boost::system::error_code& ec) = 0;

protected:
//    boost::beast::flat_buffer buffer_;
};

template<>
class Conn<false> : public StreamEntry
{
public:
    Conn(boost::asio::io_context& ioc,
         boost::asio::ssl::context& ssl,
         const kn::net::kafka::Entry& entry,
         const std::string& cmd,
         const BaseConfig& base_config,
         const std::vector<std::string>& subs)
            :StreamEntry(entry, cmd, base_config, subs)
    {


    }

    virtual ~Conn() {}

    void DoSession(boost::asio::yield_context yield)
    {
        boost::beast::error_code ec;

  START:
        Init();
        StreamEntry::Init();
        Sub(yield, ec);

        while (!stop_)
        {
            if (stop_ ) break;
            const auto& msg = Read();
            if (stop_) break;

            if (UNLIKELY(!msg)) {
                continue;
            }
            if (msg.is_eof()) {
                continue;
            }

            // Messages can contain error notifications rather than actual data
            if (UNLIKELY(bool(msg.get_error()))) {
                G3LOG(ERROR) << "Read|" << endpoint() << "|" << msg.get_error().to_string();
                continue;
            }


            {
                Ctrl ctrl = OnRead(msg, yield, ec);
                G3LOG_IF(ERROR, !!ec) << "OnRead|" << endpoint() << "|"
                                    << ec << "|" << ec.message() << "|"
                                    << (msg.get_payload().get_data());
            }
        }
        Close(yield, ec);
    }

    virtual void Init() = 0;
    virtual Ctrl OnRead(const cppkafka::Message& msg,
                        boost::asio::yield_context& yield, boost::system::error_code& ec) = 0;

protected:
//    boost::beast::flat_buffer buffer_;
};



} // ws
} // net
} // kn

#endif // _KN_NET_KAFKA_CONN_H_
