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

#ifndef _KN_TOOL_THREAD_H_
#define _KN_TOOL_THREAD_H_

#pragma once

namespace kn
{
namespace tool
{

class Thread
{
public:
    explicit Thread(int cpu) :cpu_(cpu){}

    void Start()
    {
        th_ = std::make_unique<std::thread>(&Thread::Run, this);
    }

    void Join()
    {
        if (th_) th_->join();
    }

    void Stop()
    {
        stop_ = true;
    }

    virtual int DoWork() noexcept = 0;

protected:
    void SetAffinity()
    {

    }

    void Run()
    {
        SetAffinity();

        int32_t us_sleep = 0;
        while (!stop_)
        {
            us_sleep = DoWork();
            if (stop_) return;
            if (0 < us_sleep)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(us_sleep));
            }
            else if (0 > us_sleep)
            {
                return;
            }
        }
    }

protected:
    std::unique_ptr<std::thread> th_;
    int cpu_ = -1;
    bool stop_ = false;
}; // class Thread

} // tool
} // kn

#endif // _KN_TOOL_THREAD_H_
