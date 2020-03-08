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

#ifndef _KN_TOOL_VALIDATER_H_
#define _KN_TOOL_VALIDATER_H_

#pragma once
#pragma GCC diagnostic ignored "-Wunused-function"
#include <gflags/gflags.h>
#include <kn/def/define.h>

namespace kn
{
namespace tool
{

namespace
{

bool ValidatePort(const char* flag_name, uint32_t port)
{
    return port > 0 && port <= 65535;
}

void RegisterValidatePort(const uint32_t* port)
{
    if (!gflags::RegisterFlagValidator(port, &ValidatePort))
    {
        std::cerr << "gflags regist valid port failed" << std::endl;
        abort();
    }
}

bool ValidateLogRotateSize(const char* flag_name, uint64_t size)
{
    return size > 0 && size <= 64 * def::GB;
}

void RegisterValidateLogRotateSize(const uint64_t* size)
{
    if (!gflags::RegisterFlagValidator(size, &ValidateLogRotateSize))
    {
        std::cerr << "gflags regist log rotate size failed" << std::endl;
        abort();
    }
}

} // anonymouse

} // tool
} // kn

#endif // _KN_TOOL_VALIDATER_H_
