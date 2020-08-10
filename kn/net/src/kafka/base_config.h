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

#ifndef _KN_NET_KAFKA_BASECONFIG_H_
#define _KN_NET_KAFKA_BASECONFIG_H_

#pragma once
//#include <g3log/g3log.hpp>
#include <folly/Likely.h>
#include <kn/log/logger.h>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <cppkafka/cppkafka.h>
#include <iostream>

namespace kn
{
namespace net
{
namespace kafka
{

struct BaseConfig
{
    std::string group_id_;
    bool auto_commit_{false} ;
    size_t queue_buffering_max_ms_{50}; //写入缓存时间
    size_t fetch_wait_max_ms_{10};       //读取频率
    std::string hosts_;
    std::vector<std::string> subs_;
    int partitions_{0};
    uint64_t offset_{-1};
};


} // kafka
} // net
} // kn

#endif // _KN_NET_KAFKA_CONN_H_


