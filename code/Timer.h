#pragma once

#include <functional>
#include <chrono>
#include <queue>
#include <vector>
#include <iostream>
#include <cassert>
#include <mutex>

namespace hongX 
{
    using TimeoutCallBack = std::function<void()>;   
    using Clock           = std::chrono::high_resolution_clock;
    using MS              = std::chrono::milliseconds;
    using Timestamp       = Clock::time_point;

    class HttpLink;

    // 定时器类
    class Timer 
    {
    public:
        Timer(const Timestamp& when, const TimeoutCallBack& cb)
            : expireTime_(when),
              callBack_(cb),
              delete_(false) {}
        ~Timer() {}

        // 把定时器置为删除
        void del()                      { delete_ = true;     }
        // 定时器是否是删除状态
        bool isDeleted()                { return delete_;     }
        // 获取定时器超时时间
        Timestamp getExpireTime() const { return expireTime_; }
        // 执行超时事件
        void runCallBack()              { callBack_();        }

    private:
        Timestamp       expireTime_; // 定时器超时时间
        TimeoutCallBack callBack_;   // 超时事件
        bool            delete_;     // 是否被删除

    }; // class Timer

} 


