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

#ifndef _KN_NET_WS_CONN_H_
#define _KN_NET_WS_CONN_H_

#pragma once
//#include <g3log/g3log.hpp>
#include <folly/Likely.h>
#include <kn/log/logger.h>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <random>

namespace kn
{
namespace net
{
namespace ws
{

using WsStreamSSL = boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>>;
using WsStreamTCP = boost::beast::websocket::stream<boost::beast::tcp_stream>;
using Entry = boost::asio::ip::basic_resolver_entry<boost::asio::ip::tcp>;
template <class Stream>
class WsStream : public Stream
{

};

template<>
class WsStream<WsStreamSSL> : public WsStreamSSL
{
public:
    WsStream(boost::asio::io_context& ioc, boost::asio::ssl::context& ssl)
            :WsStreamSSL(ioc, ssl)
            ,ioc_(ioc)
            ,ssl_(ssl) {}

    boost::asio::io_context& ioc_;
    boost::asio::ssl::context& ssl_;
};

template<>
class WsStream<WsStreamTCP> : public WsStreamTCP
{
public:
    explicit WsStream(boost::asio::io_context& ioc)
            :WsStreamTCP(ioc)
            ,ioc_(ioc) {}

    boost::asio::io_context& ioc_;
};

template<class Stream>
class StreamEntry
{
public:
    StreamEntry(Stream* stream,
                const Entry& entry,
                const std::string& cmd,
                const std::vector<std::string>& subs)
            :stream_(stream)
            ,entry_(entry)
            ,cmd_(cmd)
            ,subs_(subs)
    {}

    virtual ~StreamEntry()
    {
        if (nullptr != stream_)
        {
            delete stream_;
            stream_ = nullptr;
        }
    }

    boost::asio::ip::basic_endpoint<boost::asio::ip::tcp> endpoint() const
    {
        return entry_.endpoint();
    }
    Stream* stream() const
    {
        return stream_;
    }
    auto host_name() const
    {
        return entry_.host_name();
    }

    auto Read(boost::beast::flat_buffer& buffer, boost::asio::yield_context& yield, boost::beast::error_code& ec)
    {
        return stream()->async_read(buffer, yield[ec]);
    }

    void Close(boost::asio::yield_context& yield, boost::beast::error_code& ec,
               boost::beast::websocket::close_code cr = boost::beast::websocket::close_code::normal)
    {
        stream()->async_close(cr, yield[ec]);
        if (ec)
        {
            G3LOG(ERROR) << "Close|" << endpoint() << "|" << uint16_t(cr) << "|" << ec << "|" << ec.message();
            boost::beast::get_lowest_layer(*stream()).close();
        }
        else G3LOG(INFO) << "Close|" << endpoint() << "|" << uint16_t(cr);
    }

    void Close(boost::beast::error_code& ec)
    {
        stream()->close(boost::beast::websocket::close_code::normal);
        if (ec) G3LOG(ERROR) << "Close|" << endpoint() << "|" << ec << "|" << ec.message();
        else G3LOG(INFO) << "Close|" << endpoint();
    }

    void UnSub(boost::asio::yield_context& yield, boost::beast::error_code& ec)
    {

    }

    void Sub(boost::asio::yield_context& yield, boost::beast::error_code& ec)
    {
        for (const auto& sub : subs_)
        {
            if (!sub.empty())
            {
                stream()->async_write(boost::asio::buffer(sub), yield[ec]);
                if (ec)
                {
                    G3LOG(ERROR) << "Sub|" << endpoint() << "|" << sub << "|" << ec << "|" << ec.message();
                }
                else
                {
                    G3LOG(INFO) << "Sub|" << endpoint() << "|" << sub;
                }
            }
        }
    }


    void ControlCallBack(boost::beast::websocket::frame_type frame_type, boost::string_view payload)
    {
        switch (frame_type)
        {
        case boost::beast::websocket::frame_type::ping:
            G3LOG(DEBUG) << "ControlCallBack|" << endpoint() << "|ping|" << payload;
            break;
        case boost::beast::websocket::frame_type::pong:
            G3LOG(DEBUG) << "ControlCallBack|" << endpoint() << "|pong|" << payload;
            break;
        case boost::beast::websocket::frame_type::close:
            G3LOG(DEBUG) << "ControlCallBack|" << endpoint() << "|close|" << payload;
            break;
        }
    }

    void Stop()
    {
        stop_ = true;
        boost::beast::error_code ec;
        Close(ec);
    }

    void SetCmd(const std::string& cmd)
    {
        cmd_ = cmd;
    }

protected:
    boost::beast::websocket::stream_base::timeout opt_timeout_{
            std::chrono::seconds(32), // handshake
            std::chrono::seconds(8), // idle timeout
            true,
        };

    Stream* stream_{nullptr};
    const Entry& entry_;
    std::string cmd_;
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
class Conn<true> : public StreamEntry<WsStream<WsStreamSSL>>
{
public:
    Conn(boost::asio::io_context& ioc,
         boost::asio::ssl::context& ssl,
         const kn::net::ws::Entry& entry,
         const std::string& cmd,
         const std::vector<std::string>& subs)
            :StreamEntry<WsStream<WsStreamSSL>>(new WsStream<WsStreamSSL>(ioc, ssl), entry, cmd, subs)
    {
        buffer_.reserve(1 << 18);
        random_e_.seed(std::chrono::system_clock::now().time_since_epoch().count());
    }

    virtual ~Conn() {}

    void Sleep(const std::chrono::seconds seconds, boost::asio::yield_context& yield, boost::system::error_code& ec)
    {
        boost::asio::steady_timer timer(stream()->ioc_, seconds);
        timer.async_wait(yield[ec]);
    }

    void Connect(boost::asio::yield_context& yield, boost::system::error_code& ec)
    {
        auto conn = stream();

        if(! SSL_set_tlsext_host_name(stream()->next_layer().native_handle(), host_name().c_str()))
        {
            boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
            throw boost::system::system_error{ec};
        }

        conn->control_callback(std::bind(&StreamEntry<WsStream<WsStreamSSL>>::ControlCallBack, this,
                                         std::placeholders::_1, std::placeholders::_2));

        boost::beast::get_lowest_layer(*conn).expires_after(std::chrono::seconds(16));
        boost::beast::get_lowest_layer(*conn).async_connect(endpoint(), yield[ec]);
        if (ec)
        {
            if (boost::asio::ssl::error::stream_truncated == ec ||
                boost::asio::ssl::error::unspecified_system_error == ec)
            {
                G3LOG(WARNING) << "Connect|" << endpoint() << "|" << ec << "|" << ec.message();
                ec.clear();
            }
            else
            {
                boost::beast::get_lowest_layer(*conn).close();
                return;
            }
        }
    }

    void HandShake(boost::asio::yield_context& yield, boost::beast::error_code& ec)
    {
        boost::beast::get_lowest_layer(*stream()).expires_after(std::chrono::seconds(16));

        stream()->set_option(boost::beast::websocket::stream_base::decorator(
            [&](boost::beast::websocket::request_type& req) {
                req.set(boost::beast::http::field::user_agent, "kn");
            }));

        //stream()->next_layer().async_handshake(boost::asio::ssl::stream_base::client, yield[ec]);
        stream()->next_layer().handshake(boost::asio::ssl::stream_base::client, ec);
        if (ec)
        {
            boost::beast::get_lowest_layer(*stream()).close();
            return;
        }

        boost::beast::get_lowest_layer(*stream()).expires_never();

        stream()->set_option(opt_timeout_);
        stream()->async_handshake(host_name(), cmd_, yield[ec]);
        if (ec)
        {
            if (boost::asio::ssl::error::stream_truncated == ec)
            {
                G3LOG(WARNING) << "HandShake|" << ec << "|" << ec.message();
                ec.clear();
            }
            else
            {
                //if (boost::beast::websocket::error::upgrade_declined == ec)
                //Close(yield, ec);
                boost::beast::get_lowest_layer(*stream()).close();
            }
        }
    }

    void ConnAndHandShake(boost::asio::yield_context& yield, boost::beast::error_code& ec)
    {
        Connect(yield, ec);
        if (ec)
        {
            G3LOG(ERROR) << "Connect|"  << endpoint() << "|" << ec << "|" << ec.message();
            return;
        }
        G3LOG(INFO) << "Connect|" << endpoint();

        HandShake(yield, ec);
        if (!ec)
        {
            G3LOG(INFO) << "HandShake|" << endpoint();
        }
        else
        {
            G3LOG(ERROR) << "HandShake|" << endpoint() << "|" << host_name() << "|" << cmd_ << "|" << ec << "|" << ec.message();
        }
    }

    void Sleep(const std::chrono::milliseconds seconds, boost::asio::yield_context& yield, boost::system::error_code& ec)
    {
        boost::asio::steady_timer timer(stream()->ioc_, seconds);
        timer.async_wait(yield[ec]);
    }

    virtual void ReConnectRate(boost::asio::yield_context& yield, boost::system::error_code& ec)
    {
        /*
          问题：1。重连->未连上->重复之前过程
               2. 重连->连上->1s内被对端关闭->重复之前过程
          方案：1. 三个字段，上次重连时间(非连接成功时间)last_reconnect_ts_，重连失败次数connect_fail_count_，重连间隔intval(根据交易所设置)。
               2. 只有当ts_now - last_reconnect_ts_相差1分钟以上才能证明上次连接成功是有效的，此时重连失败次数清零。
               3. 重连间隔 按照失败次数累进。
        */

        uint64_t intval = 100; //ms
        auto ts_now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        std::uniform_int_distribution<uint64_t> range(0, 100);
        auto range_intval = range(random_e_);

        if ((ts_now - last_reconnect_ts_) >= 20*1000)
        {
            connect_fail_count_ = 0;
        }
        ++connect_fail_count_;
        if(connect_fail_count_ <= 2)
        {
            //Sleep(std::chrono::milliseconds(1), yield, ec);
        }
        else if(connect_fail_count_ <= 5)
        {
            Sleep(std::chrono::milliseconds((1 + range_intval / 10) * 1 * intval + range_intval % 10), yield, ec);
        }
        else if(connect_fail_count_ <= 10)
        {
            Sleep(std::chrono::milliseconds((1 + range_intval / 10) * 10 * intval + range_intval % 10), yield, ec);
        }
        else if(connect_fail_count_ <= 50)
        {
            Sleep(std::chrono::milliseconds((1 + range_intval / 10) * 50 * intval + range_intval % 10), yield, ec);
        }
        else
        {
            Sleep(std::chrono::milliseconds((1 + range_intval / 10) * 600 * intval + range_intval % 10), yield, ec);
        }
        last_reconnect_ts_ = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    virtual void ApiRate()
    {

    }

    void ReConnect(boost::asio::yield_context& yield, boost::beast::error_code& ec)
    {
        if (stream()->is_open())
        {
            Close(yield, ec);
        }
        ReConnectRate(yield, ec);

        auto& ioc = stream_->ioc_;
        auto& ssl = stream_->ssl_;
        delete stream_;
        stream_ = nullptr;
        stream_ = new std::remove_pointer_t<decltype(stream_)>(ioc, ssl);
        ConnAndHandShake(yield, ec);
        if (ec)
        {
            //if (boost::asio::error::network_unreachable == ec ||
            //         boost::asio::error::network_down == ec ||
            //         boost::asio::error::network_reset == ec)
            //{
            //    boost::beast::get_lowest_layer(*stream()).close();
            //    //std::this_thread::sleep_for(std::chrono::seconds(3));
            //}
            //if (ec != boost::asio::error::operation_aborted &&
            //    ec != boost::beast::error::timeout &&
            //    ec != boost::beast::websocket::error::closed)
            //{
            //    boost::beast::error_code err;
            //    Close(yield, err, boost::beast::websocket::close_code::abnormal);
            //}
        }
        else
        {
            Init();
            Sub(yield, ec);
        }
    }

    void DoSession(boost::asio::yield_context yield)
    {
        boost::beast::error_code ec;

  START:
        while (!stream()->is_open() && !stop_)
        {
            ReConnect(yield, ec);
        }

        while (!stop_)
        {
            if (stop_ ) break;
            auto size_read = Read(buffer_, yield, ec);
            if (stop_) break;

            if (UNLIKELY(0 != ec.value()))
            {
                buffer_.consume(size_read);
                if (boost::beast::error::timeout == ec || boost::asio::error::operation_aborted == ec)
                {
                    G3LOG(WARNING) << "Read|" << endpoint() << "|" << ec << "|" << ec.message();
                    ReConnect(yield, ec);
                }
                else
                {
                    G3LOG(ERROR) << "Read|" << endpoint() << "|" << ec << "|" << ec.message();
                    //stop_ = true;
                }
            }
            else
            {
                Ctrl ctrl = OnRead(static_cast<const char*>(buffer_.data().data()), size_read, yield, ec);
                G3LOG_IF(ERROR, !!ec) << "OnRead|" << endpoint() << "|"
                                    << ec << "|" << ec.message() << "|"
                                    << boost::beast::make_printable(buffer_.data());
                //Close(yield, ec, boost::beast::websocket::close_code::bad_payload);
                //ReConnect(cmd_, yield, ec);
                //if (boost::beast::websocket::error::upgrade_declined == ec) return;

                buffer_.consume(size_read);
                switch(ctrl)
                {
                case CtrlReConnect:
                    {
                        do
                        {
                            Close(yield, ec);
                        } while (stream()->is_open());
                        goto START;
                    }
                    break;
                case CtrlReSub:
                    {
                        UnSub(yield, ec);
                        Init();
                        Sub(yield, ec);
                    }
                    break;
                case CtrlPing:
                    stream()->ping({});
                    G3LOG_IF(ERROR, !!ec) << "Ping|" << endpoint() << "|" << ec << "|" << ec.message();
                    break;
                default:
                    break;;
                }
            }
        }
        Close(yield, ec);
    }

    virtual void Init() = 0;
    virtual Ctrl OnRead(const char* data, const size_t& size_read,
                        boost::asio::yield_context& yield, boost::system::error_code& ec) = 0;

protected:
    boost::beast::flat_buffer buffer_;
    uint64_t last_reconnect_ts_{0};
    uint64_t last_reconnect_succ_ts_{0};
    uint32_t connect_fail_count_{0};
    std::default_random_engine random_e_;
};


template<>
class Conn<false> : public StreamEntry<WsStream<WsStreamTCP>>
{
public:
    Conn(boost::asio::io_context& ioc,
         const kn::net::ws::Entry& entry,
         const std::string& cmd,
         const std::vector<std::string>& subs)
            :StreamEntry<WsStream<WsStreamTCP>>(new WsStream<WsStreamTCP>(ioc), entry, cmd, subs)
    {
        buffer_.reserve(1 << 18);
        random_e_.seed(std::chrono::system_clock::now().time_since_epoch().count());
    }

    virtual ~Conn() {}

    void Connect(boost::asio::yield_context& yield, boost::system::error_code& ec)
    {
        auto conn = stream();

        conn->control_callback(std::bind(&StreamEntry<WsStream<WsStreamTCP>>::ControlCallBack, this,
                                         std::placeholders::_1, std::placeholders::_2));

        boost::beast::get_lowest_layer(*conn).expires_after(std::chrono::seconds(30));
        boost::beast::get_lowest_layer(*conn).async_connect(endpoint(), yield[ec]);
        if (ec)
        {
            if (boost::asio::ssl::error::stream_truncated == ec ||
                boost::asio::ssl::error::unspecified_system_error == ec)
            {
                G3LOG(WARNING) << "Connect|" << endpoint() << "|" << ec << "|" << ec.message();
                ec.clear();
            }
            else
            {
                boost::beast::get_lowest_layer(*conn).close();
                return;
            }
        }
        //conn->set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::client));
    }

    void HandShake(boost::asio::yield_context& yield, boost::beast::error_code& ec)
    {
        boost::beast::get_lowest_layer(*stream()).expires_never();
        stream()->set_option(boost::beast::websocket::stream_base::decorator(
            [&](boost::beast::websocket::request_type& req) {
                req.set(boost::beast::http::field::user_agent, "kn");
            }));

        stream()->set_option(opt_timeout_);
        stream()->async_handshake(host_name(), cmd_, yield[ec]);
        if (ec)
        {
            if (boost::asio::ssl::error::stream_truncated == ec)
            {
                G3LOG(WARNING) << "HandShake|" << ec << "|" << ec.message();
                ec.clear();
            }
            else
            {
                //if (boost::beast::websocket::error::upgrade_declined == ec)
                //Close(yield, ec);
                boost::beast::get_lowest_layer(*stream()).close();
            }
        }
    }

    void ConnAndHandShake(boost::asio::yield_context& yield, boost::beast::error_code& ec)
    {
        Connect(yield, ec);
        if (ec)
        {
            G3LOG(ERROR) << "Connect|"  << endpoint() << "|" << ec << "|" << ec.message();
            return;
        }
        G3LOG(INFO) << "Connect|" << endpoint();

        HandShake(yield, ec);
        if (!ec)
        {
            G3LOG(INFO) << "HandShake|" << endpoint();
        }
        else
        {
            G3LOG(ERROR) << "HandShake|" << endpoint() << "|" << host_name() << "|" << cmd_ << "|" << ec << "|" << ec.message();
        }
    }

    void Sleep(const std::chrono::milliseconds seconds, boost::asio::yield_context& yield, boost::system::error_code& ec)
    {
        boost::asio::steady_timer timer(stream()->ioc_, seconds);
        timer.async_wait(yield[ec]);
    }

    virtual void ReConnectRate(boost::asio::yield_context& yield, boost::system::error_code& ec)
    {
        /*
          问题：1。重连->未连上->重复之前过程
               2. 重连->连上->1s内被对端关闭->重复之前过程
          方案：1. 三个字段，上次重连时间(非连接成功时间)last_reconnect_ts_，重连失败次数connect_fail_count_，重连间隔intval(根据交易所设置)。
               2. 只有当ts_now - last_reconnect_ts_相差1分钟以上才能证明上次连接成功是有效的，此时重连失败次数清零。
               3. 重连间隔 按照失败次数累进。
        */
       uint64_t intval = 100; //ms
        auto ts_now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        std::uniform_int_distribution<uint64_t> range(0, 100);
        auto range_intval = range(random_e_);

        if ((ts_now - last_reconnect_ts_) >= 20*1000)
        {
            connect_fail_count_ = 0;
        }
        ++connect_fail_count_;
        if(connect_fail_count_ <= 2)
        {
            //Sleep(std::chrono::milliseconds(1), yield, ec);
        }
        else if(connect_fail_count_ <= 5)
        {
            Sleep(std::chrono::milliseconds((1 + range_intval / 10) * 1 * intval + range_intval % 10), yield, ec);
        }
        else if(connect_fail_count_ <= 10)
        {
            Sleep(std::chrono::milliseconds((1 + range_intval / 10) * 10 * intval + range_intval % 10), yield, ec);
        }
        else if(connect_fail_count_ <= 50)
        {
            Sleep(std::chrono::milliseconds((1 + range_intval / 10) * 50 * intval + range_intval % 10), yield, ec);
        }
        else
        {
            Sleep(std::chrono::milliseconds((1 + range_intval / 10) * 600 * intval + range_intval % 10), yield, ec);
        }
        last_reconnect_ts_ = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    virtual void ApiRate()
    {

    }

    void ReConnect(boost::asio::yield_context& yield, boost::beast::error_code& ec)
    {
        if (stream()->is_open())
        {
            Close(yield, ec);
        }
        ReConnectRate(yield, ec);

        auto& ioc = stream()->ioc_;
        delete stream_;
        stream_ = new std::remove_pointer_t<decltype(stream_)>(ioc);
        ConnAndHandShake(yield, ec);
        if (ec)
        {
            //if (boost::asio::error::network_unreachable == ec ||
            //         boost::asio::error::network_down == ec ||
            //         boost::asio::error::network_reset == ec)
            //{
            //    boost::beast::get_lowest_layer(*stream()).close();
            //    //std::this_thread::sleep_for(std::chrono::seconds(3));
            //}
            //if (ec != boost::asio::error::operation_aborted &&
            //    ec != boost::beast::error::timeout &&
            //    ec != boost::beast::websocket::error::closed)
            //{
            //    boost::beast::error_code err;
            //    Close(yield, err, boost::beast::websocket::close_code::abnormal);
            //}
        }
        else
        {
            Init();
            Sub(yield, ec);
        }
    }

    void DoSession(boost::asio::yield_context yield)
    {
        boost::beast::error_code ec;

  START:
        while (!stream()->is_open() && !stop_)
        {
            ReConnect(yield, ec);
        }

        while (!stop_)
        {
            if (stop_ ) break;
            auto size_read = Read(buffer_, yield, ec);
            if (stop_) break;

            if (UNLIKELY(0 != ec.value()))
            {
                buffer_.consume(size_read);
                if (boost::beast::error::timeout == ec || boost::asio::error::operation_aborted == ec)
                {
                    G3LOG(WARNING) << "Read|" << endpoint() << "|" << ec << "|" << ec.message();
                    ReConnect(yield, ec);
                }
                else
                {
                    G3LOG(ERROR) << "Read|" << endpoint() << "|" << ec << "|" << ec.message();
                    //stop_ = true;
                }
            }
            else
            {
                Ctrl ctrl = OnRead(static_cast<const char*>(buffer_.data().data()), size_read, yield, ec);
                LOG_IF(ERROR, !!ec) << "OnRead|" << endpoint() << "|"
                                    << ec << "|" << ec.message() << "|"
                                    << boost::beast::make_printable(buffer_.data());
                //Close(yield, ec, boost::beast::websocket::close_code::bad_payload);
                //ReConnect(cmd_, yield, ec);
                //if (boost::beast::websocket::error::upgrade_declined == ec) return;

                buffer_.consume(size_read);
                if (UNLIKELY(CtrlReConnect == ctrl))
                {
                    do
                    {
                        Close(yield, ec);
                    } while (stream()->is_open());
                    goto START;
                }
            }
        }
        Close(yield, ec);
    }

    virtual void Init() = 0;
    virtual Ctrl OnRead(const char* data, const size_t& size_read,
                        boost::asio::yield_context& yield, boost::system::error_code& ec) = 0;

protected:
    boost::beast::flat_buffer buffer_;
    uint64_t last_reconnect_ts_{0};
    uint64_t last_reconnect_succ_ts_{0};
    uint32_t connect_fail_count_{0};
    std::default_random_engine random_e_;
};


} // ws
} // net
} // kn

#endif // _KN_NET_WS_CONN_H_
