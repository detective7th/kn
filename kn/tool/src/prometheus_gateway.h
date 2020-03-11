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

#ifndef _KN_TOOL_PROMETHEUS_GATEWAY_
#define _KN_TOOL_PROMETHEUS_GATEWAY_

#pragma once
//#include <prometheus/exposer.h>
#include <prometheus/gateway.h>
#include <kn/log/logger.h>
#include "thread.h"

namespace kn
{
namespace tool
{

class PrometheusGateway : public Thread
{
public:
    PrometheusGateway(const std::string& ip_port, const std::string& job, std::shared_ptr<prometheus::Registry> registry,
                      int push_interval_micro_sec = 4 * 1000000)
            :Thread(0)
            ,push_interval_micro_sec_(push_interval_micro_sec)
    {
        std::size_t colon_pos = ip_port.find(':');

        std::string ip{"3.123.252.54"};
        std::string port{"49451"};

        if (std::string::npos != colon_pos)
        {
            ip = std::string(ip_port.c_str(), colon_pos);
            port = std::string(ip_port.c_str() + colon_pos + 1);
        }
        else
        {
            ip = ip_port;
        }
        gateway_ = new prometheus::Gateway(ip, port, job, prometheus::Gateway::GetInstanceLabel(boost::asio::ip::host_name()));
        gateway_->RegisterCollectable(registry);
    }

    virtual ~PrometheusGateway()
    {
        if (gateway_)
        {
            delete gateway_;
            gateway_ = nullptr;
        }
    }

    virtual int DoWork() noexcept override
    {
        std::future<int> ret = gateway_->AsyncPush();
        int status = ret.get();
        LOG_IF(WARNING, 200 != status) << "Gateway Push Error status|" << status;
        return push_interval_micro_sec_;
    }

protected:
    prometheus::Gateway* gateway_ {nullptr};
    int push_interval_micro_sec_ {4 * 1000000};
}; // PrometheusGateway

} // tool
} // kn

#endif // _FLASH_TOOL_PROMETHEUS_GATEWAY_
