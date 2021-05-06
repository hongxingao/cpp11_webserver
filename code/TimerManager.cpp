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

    // ��ͬһ��request������������addTimer����Ҫ��ǰһ����ʱ��ɾ��
    if (link->getTimer() != nullptr)
        delTimer(link);

    link->setTimer(timer);
}

// �����������������û���̰߳�ȫ����
// ����������������Ϊ���������������������handleExpireTimers->runCallBack->__closeConnection->delTimer
void TimerManager::delTimer(HttpLink* link)
{
    // std::unique_lock<std::mutex> lock(lock_);
    assert(link != nullptr);

    Timer* timer = link->getTimer();
    if (timer == nullptr)
        return;

    // �������д��delete timeNode����ʹpriority_queue��Ķ�Ӧָ���ɴ���ָ��
    // ��ȷ�ķ����Ƕ���ɾ��
    timer->del();
    // ��ֹrequest->getTimer()���ʵ�����ָ��
    link->setTimer(nullptr);
}

void TimerManager::handleExpireTimers()
{
    std::unique_lock<std::mutex> lock(lock_);
    updateTime();
    // ������ʱ��
    while (!timerQueue_.empty()) 
    {
        Timer* timer = timerQueue_.top();
        assert(timer != nullptr);
        // ��ʱ����ɾ��
        if (timer->isDeleted()) 
        {
            timerQueue_.pop();
            delete timer;
            continue;
        }

        // һֱ�ҵ���һ����Ч�Ķ�ʱ��
        // ���ȶ���ͷ���Ķ�ʱ��Ҳû�г�ʱ��return
        if (std::chrono::duration_cast<MS>(timer->getExpireTime() - now_).count() > 0) 
            return;

        // ��ʱ
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
