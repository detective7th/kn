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

#ifndef _KN_NET_KAFKA_PARASITIFER_H_
#define _KN_NET_KAFKA_PARASITIFER_H_

#pragma once
#include "parasitism.h"
#include "conn.h"
#include "base_config.h"

namespace kn
{
namespace net
{
namespace kafka
{

//template<class WsStream> class Parasitifer;

template<bool EnableSSL>
class ParasitiferBase
{
public:
    using Conns = std::vector<Parasitism<EnableSSL>*>;

    ParasitiferBase(const std::string& domain,
                    const std::string& port,
                    uint32_t conn_count,
                    boost::asio::io_context& ioc)
            :domain_(domain),
             port_(port),
             conn_count_(conn_count),
             ioc_(ioc){}

    void MustResolve(boost::asio::io_context& ioc)
    {
        boost::system::error_code ec;
        boost::asio::ip::tcp::resolver resolver(ioc);
        //boostasio::ip::tcp::resolver::query quert(boost::asio::ip::tcp::v4(), domain_, port_);
        do
        {
            results_ = resolver.resolve(domain_, port_, ec);
            LOG_IF(WARNING, !!ec) << "Resolve|" << ec << "|" << ec.message();
            std::this_thread::sleep_for(std::chrono::seconds(3));
        } while(ec);

        G3LOG(INFO) <<  "Resolve|" << results_.size();
    }

    virtual ~ParasitiferBase()
    {
        for (auto& conn : conns_)
        {
            if (nullptr != conn)
            {
                auto c = conn->stream();
                if (nullptr != c)
                {
                    //delete c;
                    c = nullptr;
                }
            }
        }
        conns_.clear();
    }

    void Close()
    {
        for (auto& conn : conns_)
        {
            if (nullptr != conn)
            {
                conn->Stop();
            }
        }
    }

    const boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp>& results()
    {
        return results_;
    }

protected:
    const std::string& domain_;
    const std::string& port_;
    uint32_t conn_count_{0};
    Conns conns_;
    boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> results_;
    boost::asio::io_context& ioc_;
};

template<bool EnableSSL>
class Parasitifer : public ParasitiferBase<EnableSSL>
{

};

template<>
class Parasitifer<true> : public ParasitiferBase<true>
{
public:
    using Deliver = typename std::function<Parasitism<true>*(boost::asio::io_context&,
                                                             boost::asio::ssl::context&,
                                                             const Entry&,
                                                             Parasitifer<true>*)>;

    Parasitifer(const std::string& domain,
                const std::string& port,
                uint32_t conn_count,
                boost::asio::io_context& ioc,
                Deliver&& deliver)
            :ParasitiferBase<true>(domain, port, conn_count, ioc),
             deliver_(std::move(deliver))

    {
        //load_root_certificates(ctx_ssl_);
        ctx_ssl_.set_options(boost::asio::ssl::context::default_workarounds |
                             boost::asio::ssl::context::no_sslv2 |
                             boost::asio::ssl::context::no_sslv3);

    };

    void Run(boost::asio::io_context& ioc)
    {
        MustResolve(ioc);
        Connect();
    }

    virtual void Connect()
    {
        conns_.reserve(results_.size());
        uint32_t conn_count = 0;
        for (auto c_it = results_.cbegin(); results_.cend() != c_it; ++c_it, ++conn_count)
        {
            if (0 != conn_count_ && conn_count == conn_count_) break;
            if (c_it->endpoint().address().is_v6()) continue;

            auto conn = deliver_(ioc_, ctx_ssl_, *c_it, this);
            if (nullptr == conn) continue;
            conns_.push_back(conn);
            boost::asio::spawn(ioc_,
                               std::bind(&Parasitism<true>::DoSession,
                                         conn,
                                         std::placeholders::_1));
            break;
        }
    }
    boost::asio::ssl::context ctx_ssl_{boost::asio::ssl::context::tlsv12_client};
    Deliver deliver_;
    //boost::asio::ssl::context ctx_ssl_{boost::asio::ssl::context::tlsv12};
}; // Parasitifer

template<>
class Parasitifer<false> : public ParasitiferBase<false>
{
public:
    using Deliver = typename std::function<Parasitism<false>*(boost::asio::io_context&,
                                                              const Entry&,
                                                              Parasitifer<false>*)>;

    Parasitifer(const std::string& domain,
                const std::string& port,
                uint32_t conn_count,
                boost::asio::io_context& ioc,
                Deliver&& deliver)
            :ParasitiferBase<false>(domain, port, conn_count, ioc),
             deliver_(std::move(deliver)) {};

    void Run(boost::asio::io_context& ioc)
    {
        MustResolve(ioc);
        Connect();
    }

    virtual void Connect()
    {
        conns_.reserve(results_.size());
        uint32_t conn_count = 0;
        for (auto c_it = results_.cbegin(); results_.cend() != c_it; ++c_it, ++conn_count)
        {
            if (0 != conn_count_ && conn_count == conn_count_) break;
            if (c_it->endpoint().address().is_v6()) continue;

            auto conn = deliver_(ioc_, *c_it, this);

            if (nullptr == conn) continue;
            conns_.push_back(conn);
            boost::asio::spawn(ioc_,
                               std::bind(&Parasitism<false>::DoSession,
                                         conn,
                                         std::placeholders::_1));
            break;
        }
    }

    Deliver deliver_;
};

} // ws
} // net
} // kn

#endif // _KN_NET_KAFKA_PARASITIFER_H_
