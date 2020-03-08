// Copyright (C) 2020  kid Novalis <detective7th@gmail.com>
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

#ifndef _KN_LOG_SINK_OUTPUT_H_
#define _KN_LOG_SINK_OUTPUT_H_

#pragma once
#include <string>
#include <iostream>
#include <g3log/logmessage.hpp>
#include "levels.h"

namespace kn
{
namespace log
{

struct SinkOutput {

// Linux xterm color
// http://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
enum FG_Color {YELLOW = 33, RED = 31, GREEN=32, WHITE = 97};

FG_Color GetColor(const LEVELS level) const {
   if (level.value == WARNING.value) { return YELLOW; }
   if (level.value == DEBUG.value) { return GREEN; }
   if (level.value == ERROR.value) { return RED; }
   if (g3::internal::wasFatal(level)) { return RED; }

   return WHITE;
}

void ReceiveLogMessage(g3::LogMessageMover logEntry) {
   auto level = logEntry.get()._level;
   auto color = GetColor(level);

   std::cout << "\033[" << color << "m"
     << logEntry.get().toString() << "\033[m";
}

}; // SinkOutput

} // log
} // kn

#endif // _KN_LOG_SINK_OUTPUT_H_
