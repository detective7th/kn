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

#ifndef _KN_LOG_LEVELS_H_
#define _KN_LOG_LEVELS_H_

#pragma once

#include <g3log/loglevels.hpp>

namespace kn
{
namespace log
{

const LEVELS ERROR {INFO.value + 1, "ERROR"};

} // log
} // industrial_revolution

#endif // _INDUSTRIAL_REVOLUTION_LOG_LEVELS_H_
