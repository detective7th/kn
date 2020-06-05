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

#ifndef _KN_TOOL_DATETIME_H_
#define _KN_TOOL_DATETIME_H_

#pragma once

#include <chrono>
#include <date/date.h>
#include <date/tz.h>
#include "str_to_number.h"

namespace kn
{
namespace tool
{

namespace
{

template<class STR, class TimePoint>
void StrToTP(const STR& str, TimePoint& ts)
{
    std::stringstream ss;
    ss << str;
    ss >> date::parse("%FT%TZ", ts);
}

template<class TimePoint>
void StrViewToTP(const std::string_view& view, TimePoint& ts)
{
    StrToTP(view, ts);
}

template<class TimePoint, class Dur>
TimePoint UnixTsToTP(const uint64_t& ts)
{
    return TimePoint(std::chrono::duration_cast<typename TimePoint::duration>(Dur(ts)));
}

template<class TimePoint, class STR>
std::errc StrUnixTsToTP(const STR& str, TimePoint& ts)
{
    uint64_t ts_num;
    auto errc = StrToNum(str, ts_num);
    if (0 == static_cast<int>(errc))
    {
        switch (str.size())
        {
        case 10: // seconds
            ts = UnixTsToTP<TimePoint, std::chrono::seconds>(ts_num);
            return errc;
        case 13: // milliseconds
            ts = UnixTsToTP<TimePoint, std::chrono::milliseconds>(ts_num);
            return errc;
        case 16: // microseconds
            ts = UnixTsToTP<TimePoint, std::chrono::microseconds>(ts_num);
            return errc;
        case 19: // nanoseconds
            ts = UnixTsToTP<TimePoint, std::chrono::nanoseconds>(ts_num);
            return errc;
        default:
            return std::errc::invalid_argument;
        }
    }
    return errc;
}

template<class T>
concept ClockType = requires(T clock)
{
    clock.now();
};

template<ClockType Clock, class Dur>
uint64_t NowTS()
{
    return std::chrono::duration_cast<Dur>(Clock::now().time_since_epoch()).count();
    //return std::chrono::duration_cast<date::sys_time<std::chrono::microseconds>::duration>(
    //    std::chrono::system_clock::now().time_since_epoch()).count()
}
//void StrViewToUTCTime(const std::string_view& view, date::utc_time<std::chrono::microseconds>& ts)
//{
//    std::stringstream ss;
//    ss << view;
//    ss >> date::parse("%FT%TZ", ts);
//}

} // anonymous namespace

} // tool
} // kn

#endif  // _KN_TOOL_DATETIME_H_
