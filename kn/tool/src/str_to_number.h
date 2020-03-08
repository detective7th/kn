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

#ifndef _KN_TOOL_STR_TO_NUMBER_H_
#define _KN_TOOL_STR_TO_NUMBER_H_

#pragma once
#include <charconv>

namespace kn
{
namespace tool
{

namespace
{

template<class STR, class NUM> std::errc StrToNum(const STR& str, NUM& ret)
{
    return std::from_chars(str.c_str(), str.c_str() + str.size(), ret).ec;
}

template<class NUM> std::errc StrToNum(const std::string_view& str, NUM& ret)
{
    return std::from_chars(str.data(), str.data() + str.size(), ret).ec;
}

} // anonymouse namespace
} // tool
} // kn

#endif //_KN_TOOL_STR_TO_NUMBER_H_
