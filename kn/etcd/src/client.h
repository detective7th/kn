/*
 * Copyright (C) 2020  kid Novalis <detective7th@gmail.com>
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

#ifndef _KN_ETCD_CLIENT_H_
#define _KN_ETCD_CLIENT_H_

#pragma once
#include <string_view>
#include <string>
#include <kn/tool/str_to_number.h>
#include <etcd/SyncClient.hpp>

namespace kn
{
namespace etcd
{

class Client : public ::etcd::Client
{
public:
    Client(const std::string& uri) : ::etcd::Client(uri) {}

    bool GetValSync(const std::string& key, int64_t& val)
    {
        try
        {
            return 0 == static_cast<int>(kn::tool::StrToNum(this->get(key).get().value().as_string(), val));
        }
        catch (std::exception const & ex)
        {
            return false;
        }
    }

    bool GetValSync(const std::string& key, double& val)
    {
        try
        {
            val = stod(this->get(key).get().value().as_string());
            return true;
        }
        catch (std::exception const & ex)
        {
            return false;
        }
    }

    bool GetValSync(const std::string& key, std::string& val)
    {
        try
        {
            val = this->get(key).get().value().as_string();
            return true;
        }
        catch (std::exception const & ex)
        {
            return false;
        }
    }
};

} // etcd
} // kn

#endif //_KN_ETCD_ETCD_CLIENT_H_
