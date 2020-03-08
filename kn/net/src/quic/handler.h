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

#ifndef _KN_NET_QUIC_HANDLER_H_
#define _KN_NET_QUIC_HANDLER_H_

#pragma once

#include <folly/io/async/EventBase.h>
#include <quic/codec/Types.h>
#include <quic/api/QuicSocket.h>
#include <kn/log/logger.h>

namespace kn
{
namespace net
{
namespace quic
{

class Handler
        : public ::quic::QuicSocket::ConnectionCallback,
          public ::quic::QuicSocket::ReadCallback,
          public ::quic::QuicSocket::WriteCallback
{
public:
    using StreamData = std::pair<folly::IOBufQueue, bool>;

    explicit Handler(folly::EventBase* evb) : evb_(evb){}

    void set_socket(std::shared_ptr<::quic::QuicSocket> socket)
    {
        socket_ = socket;
    }

    void onNewBidirectionalStream(::quic::StreamId id) noexcept override
    {
        LOG(INFO) << "Got bidirectional stream id=" << id;
        socket_->setReadCallback(id, this);
    }

    void onNewUnidirectionalStream(::quic::StreamId id) noexcept override
    {
        LOG(INFO) << "Got unidirectional stream id=" << id;
        socket_->setReadCallback(id, this);
    }

    void onStopSending(::quic::StreamId id, ::quic::ApplicationErrorCode error) noexcept override
    {
        LOG(INFO) << "Got StopSending stream id=" << id << " error=" << error;
    }

    void onConnectionEnd() noexcept override
    {
        LOG(INFO) << "Socket closed";
    }

    void onConnectionError(std::pair<::quic::QuicErrorCode, std::string> error) noexcept override
    {
        LOG(ERROR) << "Socket error=" << ::quic::toString(error.first);
    }

    void readAvailable(::quic::StreamId id) noexcept override
    {
        LOG(INFO) << "read available for stream id=" << id;
    }

    void readError(::quic::StreamId id, std::pair<::quic::QuicErrorCode,
                   folly::Optional<folly::StringPiece>> error) noexcept override
    {
        LOG(ERROR) << "Got read error on stream=" << id
                   << " error=" << toString(error);
        // A read error only terminates the ingress portion of the stream state.
        // Your application should probably terminate the egress portion via
        // resetStream
    }

    void onStreamWriteReady(::quic::StreamId id, uint64_t maxToSend) noexcept override
    {
        LOG(INFO) << "socket is write ready with maxToSend=" << maxToSend;
    }

    void onStreamWriteError(::quic::StreamId id,
                            std::pair<::quic::QuicErrorCode, folly::Optional<folly::StringPiece>> error) noexcept override
    {
        LOG(ERROR) << "write error with stream=" << id
                   << " error=" << toString(error);
    }

    folly::EventBase* evb()
    {
        return evb_;
    }

protected:
    folly::EventBase* evb_;
    std::shared_ptr<::quic::QuicSocket> socket_;
    std::map<::quic::StreamId, StreamData> input_;
}; // Handler

} // quic
} // net
} // kn

#endif // _KN_NET_QUIC_HANDLER_H_
