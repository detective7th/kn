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

#ifndef _KN_TOOL_AWS_H_
#define _KN_TOOL_AWS_H_
#define CPPHTTPLIB_OPENSSL_SUPPORT

#pragma once

#include <chrono>
#include <date/date.h>
#include <date/tz.h>
#include "str_to_number.h"
#include <httplib.h>

namespace kn
{
namespace tool
{

namespace
{

int GetAwsPublicIpv4(std::string& output) noexcept {
    std::string path("/latest/meta-data/public-ipv4");

    httplib::Client cli("instance-data", 80);
    auto res = cli.Get(path.c_str());
    if (!res)
    {
        return -1;
    }
    if (res->status == 0) {
        return -2;
    }
    if (res->status != 200) {
        return res->status;
    }
    output = res->body;
    return 0;
}


} // anonymous namespace

} // tool
} // kn

#endif  // _KN_TOOL_AWS_H_
