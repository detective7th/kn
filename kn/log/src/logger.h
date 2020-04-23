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

#ifndef _KN_LOG_LOGGER_H_
#define _KN_LOG_LOGGER_H_

#pragma once

#include <string>
#include <vector>
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>
#include <g3sinks/LogRotateWithFilter.h>
#include "levels.h"
#ifdef DDEBUG
#include "sink_output.h"
#endif

namespace kn
{
namespace log
{

class Logger
{
public:
    explicit Logger(std::string filename, std::string prefix, std::size_t rotate_size)
    {
        std::string commond("mkdir -p " + prefix);
        system(commond.c_str());

        g3::only_change_at_initialization::addLogLevel(ERROR, true);
#ifndef DDEBUG
        g3::log_levels::disable(DEBUG);
#endif // DDEBUG
        g3::restoreSignalHandler(SIGTERM);
        g3::restoreSignalHandler(SIGQUIT);
        g3::restoreSignalHandler(SIGINT);

        worker_ = g3::LogWorker::createLogWorker();

        g3::initializeLogging(worker_.get());
        std::vector<LEVELS> filter;
        sink_rotate_ = worker_->addSink(LogRotateWithFilter::CreateLogRotateWithFilter(filename, prefix, filter)
                                        , &LogRotateWithFilter::save);

        sink_rotate_->call(&LogRotateWithFilter::overrideLogDetails, g3::LogMessage::FullLogDetailsToString).wait();
        sink_rotate_->call(&LogRotateWithFilter::setMaxLogSize, rotate_size).wait();
        sink_rotate_->call(&LogRotateWithFilter::setFlushPolicy, 0).wait();

#ifdef DDEBUG
        sink_output_ = worker_->addSink(std::make_unique<SinkOutput>(), &SinkOutput::ReceiveLogMessage);
#endif // DDEBUG

    }

    void Flush()
    {
        sink_rotate_->call(&LogRotateWithFilter::flush).wait();
    }

protected:
    std::unique_ptr<g3::LogWorker> worker_ = nullptr;
    std::unique_ptr<g3::SinkHandle<LogRotateWithFilter>> sink_rotate_ = nullptr;
#ifdef DDEBUG
    std::unique_ptr<g3::SinkHandle<SinkOutput>> sink_output_ = nullptr;
#endif // DDEBUG
};

} // log
} // kn

using namespace kn::log;

// LOG(level) is the API for the stream log
#define G3LOG(level) if(!g3::logLevel(level)) {} else INTERNAL_LOG_MESSAGE(level).stream()


// 'Conditional' stream log
#define G3LOG_IF(level, boolean_expression)  \
    if (false == (boolean_expression) || !g3::logLevel(level)) {} else INTERNAL_LOG_MESSAGE(level).stream()

#endif // _KN_LOG_LOGGER_H_
