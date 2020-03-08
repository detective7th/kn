// <one line to give the program's name and a brief idea of what it does.>
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

#ifndef _KN_NET_QUIC_SRV_H_
#define _KN_NET_QUIC_SRV_H_

#pragma once

#include <folly/io/async/EventBase.h>
#include <quic/server/QuicServer.h>
#include "handler.h"

namespace kn
{
namespace net
{
namespace quic
{

class SrvTransportFactory : public ::quic::QuicServerTransportFactory
{
public:
    SrvTransportFactory() {}
    virtual ~SrvTransportFactory() override
    {
        while (!handlers_.empty())
        {
            auto& handler = handlers_.back();
            handler->evb()->runImmediatelyOrRunInEventBaseThreadAndWait(
                [this] {
                    // The evb should be performing a sequential consistency atomic
                    // operation already, so we can bank on that to make sure the writes
                    // propagate to all threads.
                    handlers_.pop_back();
                });
        }
    }

    ::quic::QuicServerTransport::Ptr make(
        folly::EventBase* evb,
        std::unique_ptr<folly::AsyncUDPSocket> socket,
        const folly::SocketAddress&,
        std::shared_ptr<const fizz::server::FizzServerContext> ctx) noexcept override
    {
        CHECK(socket->getEventBase() == evb);
        auto handler = std::make_unique<Handler>(evb);
        auto transport = ::quic::QuicServerTransport::make(evb, std::move(socket), *handler, ctx);
        handler->set_socket(transport);
        handlers_.push_back(std::move(handler));
        return transport;
    }

protected:
    std::vector<std::unique_ptr<Handler>> handlers_;
};

class Srv
{
public:
    Srv() : srv_(::quic::QuicServer::createQuicServer())
    {
        srv_->setQuicServerTransportFactory(std::make_unique<SrvTransportFactory>());
        ::quic::TransportSettings settings;
        settings.partialReliabilityEnabled = true;
        srv_->setTransportSettings(settings);
    }

    void Start()
    {
        // Create a SocketAddress and the default or passed in host.
        folly::SocketAddress addr1("0.0.0.0", 6666);
        addr1.setFromHostPort("0.0.0.0", 6666);
        srv_->start(addr1, 0);
        eventbase_.loopForever();
    }

protected:
    folly::EventBase eventbase_;
    std::shared_ptr<::quic::QuicServer> srv_;
};

} // quic
} // net
} // kn

#endif // _KN_NET_QUIC_SRV_H_
