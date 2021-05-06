#include "TimerManager.h"
#include "HttpLink.h"

#include <cassert>

using namespace hongX;

void TimerManager::addTimer(HttpLink* link, const int& timeout, const TimeoutCallBack& cb)
{
    std::unique_lock<std::mutex> lock(lock_);
    assert(link != nullptr);

    updateTime();
    Timer* timer = new Timer(now_ + MS(timeout), cb);
    timerQueue_.push(timer);

    // 对同一个request连续调用两次addTimer，需要把前一个定时器删除
    if (link->getTimer() != nullptr)
        delTimer(link);

    link->setTimer(timer);
}

// 这个函数不必上锁，没有线程安全问题
// 若上锁，反而会因为连续两次上锁造成死锁：handleExpireTimers->runCallBack->__closeConnection->delTimer
void TimerManager::delTimer(HttpLink* link)
{
    // std::unique_lock<std::mutex> lock(lock_);
    assert(link != nullptr);

    Timer* timer = link->getTimer();
    if (timer == nullptr)
        return;

    // 如果这里写成delete timeNode，会使priority_queue里的对应指针变成垂悬指针
    // 正确的方法是惰性删除
    timer->del();
    // 防止request->getTimer()访问到垂悬指针
    link->setTimer(nullptr);
}

void TimerManager::handleExpireTimers()
{
    std::unique_lock<std::mutex> lock(lock_);
    updateTime();
    // 遍历定时器
    while (!timerQueue_.empty()) 
    {
        Timer* timer = timerQueue_.top();
        assert(timer != nullptr);
        // 定时器被删除
        if (timer->isDeleted()) 
        {
            timerQueue_.pop();
            delete timer;
            continue;
        }

        // 一直找到第一个有效的定时器
        // 优先队列头部的定时器也没有超时，return
        if (std::chrono::duration_cast<MS>(timer->getExpireTime() - now_).count() > 0) 
            return;

        // 超时
        timer->runCallBack();
        timerQueue_.pop();
        delete timer;
    }
}

int TimerManager::getNextExpireTime()
{
    std::unique_lock<std::mutex> lock(lock_);
    updateTime();
    int res = -1;
    while (!timerQueue_.empty()) {
        Timer* timer = timerQueue_.top();
        if (timer->isDeleted()) {
            timerQueue_.pop();
            delete timer;
            continue;
        }
        res = std::chrono::duration_cast<MS>(timer->getExpireTime() - now_).count();
        res = (res < 0) ? 0 : res;
        break;
    }
    return res;
}
