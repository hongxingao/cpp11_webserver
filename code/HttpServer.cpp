#include "HttpServer.h"
#include "HttpLink.h"
#include "HttpResponse.h"
#include "Utils.h"
#include "Epoll.h"
#include "ThreadPool.h"
#include "TimerManager.h"

#include <iostream>
#include <functional> 
#include <cassert> 
#include <cstring> 
#include <unistd.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 

using namespace hongX;

HttpServer::HttpServer(int port, int numThread) 
    : port_(port),
      listenFd_(createListenFd(port_)),
      listenLink_(new HttpLink(listenFd_)),
      epoll_(new Epoll()),
      threadPool_(new ThreadPool(numThread)),
      timerManager_(new TimerManager())
{
    assert(listenFd_ >= 0);
}

HttpServer::~HttpServer()
{
}

void HttpServer::run()
{
    // 注册监听套接字到epoll（可读事件，ET模式）
    epoll_->operFd(n_type_operate_fd_add, listenFd_, listenLink_.get(), (EPOLLIN | EPOLLET));
    // 注册新连接回调函数
    epoll_->setOnConnect(std::bind(&HttpServer::__acceptConnection, this));
    // 注册关闭连接回调函数
    epoll_->setOnDisConnect(std::bind(&HttpServer::__closeConnection, this, std::placeholders::_1));
    // 注册请求处理回调函数
    epoll_->setOnRequest(std::bind(&HttpServer::__doRequest, this, std::placeholders::_1));
    // 注册响应处理回调函数
    epoll_->setOnResponse(std::bind(&HttpServer::__doResponse, this, std::placeholders::_1));

    // 事件循环
    while(1) 
    {
        // 获取最快要超时的定时器时间
        int timeMS = timerManager_->getNextExpireTime();

        // 等待事件发生
        int eventsNum = epoll_->wait(timeMS);

        // 分发事件处理函数
        if(eventsNum > 0) 
            epoll_->handleEvent(listenFd_, threadPool_, eventsNum);
        
        // 检查超时
        timerManager_->handleExpireTimers();   
    }
}

// 接收连接 ET模式
void HttpServer::__acceptConnection()
{
    while(1) 
    {
        int acceptFd = accept4(listenFd_, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if(acceptFd == -1) 
        {
            if(errno == EAGAIN)
                break;
            LOG_INFO("[HttpServer::__acceptConnection] accept : %s\n", strerror(errno));
            break;
        }

        // 为新的连接套接字分配HttpLink资源
        HttpLink* link = new HttpLink(acceptFd);
        timerManager_->addTimer(link, g_connect_timeout, std::bind(&HttpServer::__closeConnection, this, link));

        // 注册连接套接字到epoll（可读，水平触发，保证任一时刻只被一个线程处理）
        epoll_ ->operFd(n_type_operate_fd_add, acceptFd, link, (EPOLLIN | EPOLLONESHOT));
    }
}

void HttpServer::__closeConnection(HttpLink* request)
{
    int fd = request->fd();
   
    //LOG_INFO("[HttpServer::__closeConnection] connect fd = %d is closed\n", fd);

    timerManager_->delTimer(request);
    epoll_->operFd(n_type_operate_fd_del, fd, request, 0);
    // 释放该套接字占用的HttpLink资源，在析构函数中close(fd)
    delete request;
    request = nullptr;
}

// 处理请求 LT模式
void HttpServer::__doRequest(HttpLink* request)
{
    timerManager_->delTimer(request);

    assert(request != nullptr);
    int fd = request->fd();

    int readErrno;
    int nRead = request->read(&readErrno);
    //LOG_INFO("recv %d bytes\n", nRead);
    
    // read返回0表示客户端断开连接
    if (nRead == 0)
    {
        __closeConnection(request);
        return;
    }

    // 非EAGAIN错误，断开连接
    if(nRead < 0 && (readErrno != EAGAIN)) 
    {
        LOG_ERROR("read error:%d\n", nRead);
        __closeConnection(request);
        return; 
    }

    // EAGAIN错误则释放线程使用权，并监听下次可读事件epoll_->mod(...)
    if(nRead < 0 && readErrno == EAGAIN) 
    {
        epoll_->operFd(n_type_operate_fd_mod, fd, request, (EPOLLIN | EPOLLONESHOT));
        timerManager_->addTimer(request, g_connect_timeout, std::bind(&HttpServer::__closeConnection, this, request));
        return;
    }

    // 解析报文，出错则断开连接
    if(!request->parseRequest()) 
    {
        // 发送400报文
        HttpResponse response(400, "", false);
        request->appendOutBuffer(response.makeResponse());

        // 立刻关闭连接了，所以就算没写完也只能写一次？
        int writeErrno;
        request->write(&writeErrno);
        __closeConnection(request); 
        return; 
    }

    // 解析完成
    if(request->parseFinish()) 
    {
        HttpResponse response(200, request->getPath(), request->keepAlive());

        request->appendOutBuffer(response.makeResponse());
        epoll_->operFd(n_type_operate_fd_mod, fd, request, (EPOLLIN | EPOLLOUT | EPOLLONESHOT));
    }
}

// LT模式
void HttpServer::__doResponse(HttpLink* request)
{
    timerManager_->delTimer(request);
    assert(request != nullptr);
    int fd = request->fd();

    int toWrite = request->writableBytes();
    //LOG_INFO("send %d bytes\n", toWrite);
    if(toWrite == 0) 
    {
        epoll_->operFd(n_type_operate_fd_mod, fd, request, (EPOLLIN | EPOLLONESHOT));
       
        timerManager_->addTimer(request, g_connect_timeout, std::bind(&HttpServer::__closeConnection, this, request));
        return;
    }

    int writeErrno;
    int ret = request->write(&writeErrno);

    if(ret < 0 && writeErrno == EAGAIN) 
    {
        epoll_->operFd(n_type_operate_fd_mod, fd, request, (EPOLLIN | EPOLLOUT | EPOLLONESHOT));
        return;
    }

    // 非EAGAIN错误，断开连接
    if(ret < 0 && (writeErrno != EAGAIN)) 
    {
        __closeConnection(request);
        return; 
    }

    // 完全写完
    if(ret == toWrite) 
    {
        if(request->keepAlive()) 
        {
            request->resetParse();
            epoll_->operFd(n_type_operate_fd_mod, fd, request, (EPOLLIN | EPOLLONESHOT));
            timerManager_->addTimer(request, g_connect_timeout, std::bind(&HttpServer::__closeConnection, this, request));
        } 
        else 
           __closeConnection(request);
        
       return;
    }

    // 未写完
    epoll_->operFd(n_type_operate_fd_mod, fd, request, (EPOLLIN | EPOLLOUT | EPOLLONESHOT));
    timerManager_->addTimer(request, g_connect_timeout, std::bind(&HttpServer::__closeConnection, this, request));

    return;
}
