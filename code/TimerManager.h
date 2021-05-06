#pragma once
#include "Timer.h"

namespace hongX
{
    // �ȽϺ���������priority_queue��ʱ��ֵ��С���ڶ�ͷ
    struct cmp 
    {
        bool operator()(Timer* a, Timer* b)
        {
            assert(a != nullptr && b != nullptr);
            return (a->getExpireTime()) > (b->getExpireTime());
        }
    };

    // ��ʱ��������
    class TimerManager {
    public:
        TimerManager() : now_(Clock::now()) {}
        ~TimerManager() {}
     
        void addTimer(HttpLink* link, const int& timeout, const TimeoutCallBack& cb); // �������Ӷ�ʱ�¼� timeout��λms
        void delTimer(HttpLink* link);             // ɾ�����Ӷ�ʱ�¼�
        void handleExpireTimers();                 // ����ʱ�¼�
        int getNextExpireTime();                   // ���س�ʱʱ��(���ȶ��������糬ʱʱ��͵�ǰʱ���)
        void updateTime() { now_ = Clock::now(); } // ����ʱ��

    private:
        using TimerQueue = std::priority_queue<Timer*, std::vector<Timer*>, cmp>;

        TimerQueue timerQueue_; // ��ʱ��С����
        Timestamp  now_;        // ��ǰʱ��
        std::mutex lock_;       // С������

    }; 

}
