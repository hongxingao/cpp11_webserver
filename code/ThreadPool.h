#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace hongX 
{
    // �̳߳���
    class ThreadPool 
    {
    public:
        using JobFunction = std::function<void()>;  // ����

        ThreadPool(int numWorkers);
        ~ThreadPool();

        // ������ѹ�����
        void pushJob(const JobFunction& job);

    private:
        std::vector<std::thread> threads_; // �߳��б�
        std::mutex               lock_;    // ���������
        std::condition_variable  cond_;    // �����������뻥�������ʵ������ -�����ѣ�
        std::queue<JobFunction>  jobs_;    // �������
        bool                     stop_;    // �Ƿ�ֹͣ
    };

}

