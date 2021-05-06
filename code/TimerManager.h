#pragma once
#include "Timer.h"

namespace hongX
{
    // 比较函数，用于priority_queue，时间值最小的在队头
    struct cmp 
    {
        bool operator()(Timer* a, Timer* b)
        {
            assert(a != nullptr && b != nullptr);
            return (a->getExpireTime()) > (b->getExpireTime());
        }
    };

    // 定时器管理类
    class TimerManager {
    public:
        TimerManager() : now_(Clock::now()) {}
        ~TimerManager() {}
     
        void addTimer(HttpLink* link, const int& timeout, const TimeoutCallBack& cb); // 插入连接定时事件 timeout单位ms
        void delTimer(HttpLink* link);             // 删除连接定时事件
        void handleExpireTimers();                 // 处理超时事件
        int getNextExpireTime();                   // 返回超时时间(优先队列中最早超时时间和当前时间差)
        void updateTime() { now_ = Clock::now(); } // 更新时间

    private:
        using TimerQueue = std::priority_queue<Timer*, std::vector<Timer*>, cmp>;

        TimerQueue timerQueue_; // 定时器小根堆
        Timestamp  now_;        // 当前时间
        std::mutex lock_;       // 小根堆锁

    }; 

}
