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

#pragma once
//#include <g3log/g3log.hpp>
#include <folly/Likely.h>
#include <kn/log/logger.h>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <cppkafka/cppkafka.h>
#include <iostream>
#include <tuple>
#include <algorithm>

namespace kn
{
namespace net
{
namespace kafka
{

class BaseConfig
{
public:
    BaseConfig(const BaseConfig &) = delete;

    BaseConfig() : config(),
    auto_commit_(false), queue_buffering_max_ms_(50), fetch_wait_max_ms_(10),
    do_manual_commit_(false)
    {
        config.set("enable.auto.commit", this->auto_commit_);
        config.set("fetch.wait.max.ms", this->fetch_wait_max_ms_);
    }

    inline void EnableManualCommit() { this->do_manual_commit_ = true; }

    inline bool IsManualCommitAllowed() { return this->do_manual_commit_; }

    void SetAutoCommit(bool am)
    {
        auto_commit_ = am;
        config.set("enable.auto.commit", am);
    }

    void SetQueueBufferingMaxMs(size_t ms)
    {
        queue_buffering_max_ms_ = ms;
        config.set("queue.buffering.max.ms", ms); // not used in original cppkafka::configuration
    }

    void SetWaitMaxMs(size_t fwmm)
    {
        fetch_wait_max_ms_ = fwmm;
        config.set("fetch.wait.max.ms", fwmm);
    }

    void SetGroupId(const std::string & kgid)
    {
        group_id_ = kgid;
        config.set("group.id", kgid);
    }

    void SetHosts(const std::string& hosts)
    {
        hosts_ = hosts;
        config.set("metadata.broker.list", hosts);
    }

    void ResetSubscribeTopics(const std::vector<std::string>& topics)
    {
        this->tpo_info_.clear();
        for(auto& t:topics)
        {
            this->tpo_info_.emplace(t, cppkafka::TopicPartition(t, 0, -1));
        }
    }

    void ResetSubscribeTopics(const std::vector<std::tuple<std::string, int, uint64_t>>& subs_info)
    {
        this->tpo_info_.clear();
        for (auto &sub : subs_info)
        {
            this->tpo_info_.emplace(std::get<0>(sub),
                cppkafka::TopicPartition(std::get<0>(sub), std::get<1>(sub), std::get<2>(sub)));
        }
    }

    void ExtendTopicsWithDefaultOffset(const std::vector<std::string> topics)
    {
        for(auto& t : topics)
        {
            auto it = this->tpo_info_.find(t);
            if (it == tpo_info_.end())
            {
                tpo_info_[t] = std::move(cppkafka::TopicPartition(t, 0, -1));
            }
        }
    }

    // void SetPartitions: always 0 for now //
    void UniversalUpdateOffset(uint64_t o)
    {
        for(auto& e: tpo_info_)
        {
            e.second.set_offset(o);
        }
    }

    void UpdateOffsets(const std::vector<std::tuple<std::string, uint64_t>>& topic_offsets, bool update_topics=true)
    {
        for(auto& to : topic_offsets)
        {
            auto it = tpo_info_.find(std::get<0>(to));
            if(it != tpo_info_.end())
            {
                it->second.set_offset(std::get<1>(to));
            }
            else
            {
                if(update_topics) // add to info
                {// with default partition 0
                    tpo_info_[std::get<0>(to)] = std::move(cppkafka::TopicPartition(std::get<0>(to), 0, std::get<1>(to)));
                }
            }

        }
    }

    std::unordered_map<std::string, uint64_t> GetOffsets() const
    {
        std::unordered_map<std::string, uint64_t> ret;
        for(auto& e : tpo_info_)
        {
            ret.emplace(e.first, e.second.get_offset());
        }
        return ret;
    }

    std::vector<std::string> GetTopics() const
    {
        std::vector<std::string> topics;
        std::transform(tpo_info_.begin(), tpo_info_.end(), std::back_inserter(topics),
                       [&](auto &e) { return e.first; });
        return topics;
    }

    std::vector<cppkafka::TopicPartition> GetTopicPartitions() const
    {
        std::vector<cppkafka::TopicPartition> ret;
        std::transform(tpo_info_.begin(), tpo_info_.end(), std::back_inserter(ret),
                       [&](auto &e) { return e.second; });
        return ret;
    }

    std::string GetGroupId() {return this->group_id_;}
    bool GetAutoCommit() {return this->auto_commit_;}
    size_t GetQueueBufferingMaxMs() { return this->queue_buffering_max_ms_; }
    size_t GetFetchWaitMaxMs() { return this->fetch_wait_max_ms_; }
    std::string GetHosts() { return this->hosts_; }

public:
    cppkafka::Configuration config;
private:
    std::string hosts_{};
    std::string group_id_{};

    bool auto_commit_;
    size_t queue_buffering_max_ms_; //写入缓存时间
    size_t fetch_wait_max_ms_;       //读取频率

    // std::vector<std::string> subs_{};
    // int partitions_{0};
    // uint64_t offset_{-1};

    bool do_manual_commit_;

    std::unordered_map<std::string, cppkafka::TopicPartition> tpo_info_;

};

} // kafka
} // net
} // kn


