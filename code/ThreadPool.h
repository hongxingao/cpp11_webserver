#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace hongX 
{
    // 线程池类
    class ThreadPool 
    {
    public:
        using JobFunction = std::function<void()>;  // 任务

        ThreadPool(int numWorkers);
        ~ThreadPool();

        // 把任务压入队列
        void pushJob(const JobFunction& job);

    private:
        std::vector<std::thread> threads_; // 线程列表
        std::mutex               lock_;    // 任务队列锁
        std::condition_variable  cond_;    // 条件变量（与互斥量配合实现生产 -》消费）
        std::queue<JobFunction>  jobs_;    // 任务队列
        bool                     stop_;    // 是否停止
    };

}

