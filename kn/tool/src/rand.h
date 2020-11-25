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

#ifndef _KN_TOOL_RAND_H_
#define _KN_TOOL_RAND_H_

#pragma once
#include <type_traits>
#include <time.h>

namespace kn
{
namespace tool
{
namespace
{

enum RandRangeType
{
    None = 0,
    CloseOpen = 1,
    CloseClose = 2,
    OpenClose = 3,
    OpenOpen = 4,
};

template<class Integer, RandRangeType RangeType>
class RandInt
{
    static_assert(std::is_integral_v<Integer>, "must be integral");
public:
    Integer operator()(const Integer& begin, const Integer& end) const
    {
        return Integer(0);
    }
};

template<class Integer>
class RandInt<Integer, CloseOpen>
{
    static_assert(std::is_integral_v<Integer>, "must be integral");
public:
    Integer operator()(const Integer& begin, const Integer& end) const
    {
        if (begin < end)
        {
            assert(end < RAND_MAX);
            return (std::rand() % (end - begin)) + begin;
        }
        else if (begin > end)
        {
            assert(begin < RAND_MAX);
            return (std::rand() % (begin - end)) + end;
        }
        else
        {
            assert(false);
        }
    }
};

template<class Integer>
class RandInt<Integer, CloseClose>
{
    static_assert(std::is_integral_v<Integer>, "must be integral");
public:
    Integer operator()(const Integer& begin, const Integer& end) const
    {
        if (begin < end)
        {
            assert(end < RAND_MAX);
            return (std::rand() % (end - begin + 1)) + begin;
        }
        else if (begin > end)
        {
            assert(begin < RAND_MAX);
            return (std::rand() % (begin - end + 1)) + end;
        }
        else
        {
            return begin;
        }
    }
};

template<class Integer>
class RandInt<Integer, OpenOpen>
{
    static_assert(std::is_integral_v<Integer>, "must be integral");
public:
    Integer operator()(const Integer& begin, const Integer& end) const
    {
        if (begin < end)
        {
            assert(end < RAND_MAX);
            return (std::rand() % (end - begin)) + begin + 1;
        }
        else if (begin > end)
        {
            assert(begin < RAND_MAX);
            return (std::rand() % (begin - end)) + end + 1;
        }
        else
        {
            assert(false);
        }
    }
};

template<class Integer>
class RandInt<Integer, OpenClose>
{
    static_assert(std::is_integral_v<Integer>, "must be integral");
public:
    Integer operator()(const Integer& begin, const Integer& end) const
    {
        if (begin < end)
        {
            assert(end < RAND_MAX);
            return (std::rand() % (end - begin + 1)) + begin + 1;
        }
        else if (begin > end)
        {
            assert(begin < RAND_MAX);
            return (std::rand() % (begin - end + 1)) + end + 1;
        }
        else
        {
            assert(false);
        }
    }
};

} // anonymouse
} // tool
} // kn

#endif // _KN_TOOL_RAND_H_
