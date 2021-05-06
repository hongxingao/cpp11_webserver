#include "ThreadPool.h"
#include <iostream>
#include <cassert>

using namespace hongX;

ThreadPool::ThreadPool(int numWorkers)
    : stop_(false)
{
    numWorkers = numWorkers <= 0 ? 1 : numWorkers;
    for(int i = 0; i < numWorkers; ++i)
        threads_.emplace_back([this]() 
        {
            while(1) 
            {
                JobFunction func = nullptr;
                {
                    std::unique_lock<std::mutex> lock(lock_);   

                    // 阻塞等待任务队列不空
                    while(!stop_ && jobs_.empty())
                        cond_.wait(lock);
                    if(jobs_.empty() && stop_) 
                        return;
                    
                    if(!jobs_.empty()) 
                    {
                        func = jobs_.front();
                        jobs_.pop();
                    }
                }
                if(func != nullptr) 
                    func();

            }
        });
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(lock_);
        stop_ = true;
    } 
    cond_.notify_all();
    for(auto& thread: threads_)
        thread.join();
}

// 把任务压入队列
void ThreadPool::pushJob(const JobFunction& job)
{
    {
        std::unique_lock<std::mutex> lock(lock_);
        jobs_.push(job);
    }
    cond_.notify_one();
}
