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

#ifndef _KN_NET_KAFKA_PARASITISM_H_
#define _KN_NET_KAFKA_PARASITISM_H_

#pragma once

#include "conn.h"
#include "base_config.h"
//#include "parasitifer.h"

namespace kn
{
namespace net
{
namespace kafka
{

template<bool EnabeSSL> class Parasitifer;

template<bool EnableSSL> class Parasitism : public Conn<EnableSSL>
{

};

template<>
class Parasitism<true> : public Conn<true>
{
public:
    Parasitism(boost::asio::io_context& ioc,
               boost::asio::ssl::context& ssl,
               const kn::net::kafka::Entry& entry,
               const std::string& cmd,
               const BaseConfig& base_config,
               const std::vector<std::string>& sub,
               Parasitifer<true>* parasitifer)
            :Conn<true>(ioc, ssl, entry, cmd, base_config, sub)
            ,parasitifer_(parasitifer)
    {}
protected:
    Parasitifer<true>* parasitifer_{nullptr};
};

template<>
class Parasitism<false> : public Conn<false>
{
public:
    Parasitism(boost::asio::io_context& ioc,
               boost::asio::ssl::context& ssl,
               const kn::net::kafka::Entry& entry,
               const std::string& cmd,
               const BaseConfig& base_config,
               const std::vector<std::string>& sub,
               Parasitifer<false>* parasitifer)
            :Conn<false>(ioc, ssl, entry, cmd, base_config, sub)
            ,parasitifer_(parasitifer)
    {}
protected:
    Parasitifer<false>* parasitifer_{nullptr};
};

} // ws
} // net
} // kn

#endif // _KN_NET_KAFKA_PARASITISM_H_
