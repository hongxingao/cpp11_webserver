#pragma once

#include <memory> // unique_ptr
#include <mutex>

namespace hongX
{
    class HttpLink;      //连接类
    class Epoll;         // epoll封装类
    class ThreadPool;    // 线程池类
    class TimerManager;  // 定时器管理类

    class HttpServer 
    {
    public:
        HttpServer(int port, int numThread);
        ~HttpServer();
        void run(); // 启动HTTP服务器
    
    private:
        void __acceptConnection();                 // 接受新连接
        void __closeConnection(HttpLink* request); // 关闭连接
        void __doRequest(HttpLink* request);       // 处理HTTP请求报文，这个函数由线程池调用
        void __doResponse(HttpLink* request);      // 处理HTTP响应报文

    private:
        using ListenRequestPtr  = std::unique_ptr<HttpLink>;
        using EpollPtr          = std::unique_ptr<Epoll>;
        using ThreadPoolPtr     = std::shared_ptr<ThreadPool>;
        using TimerManagerPtr   = std::unique_ptr<TimerManager>;

        int              port_;         // 监听端口
        int              listenFd_;     // 监听套接字
        ListenRequestPtr listenLink_;   // 监听套接字的HttpLink实例
        EpollPtr         epoll_;        // epoll实例
        ThreadPoolPtr    threadPool_;   // 线程池
        TimerManagerPtr  timerManager_; // 定时器管理器
    };

} 

