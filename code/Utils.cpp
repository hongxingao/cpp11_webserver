#include "Utils.h"
#include <iostream>
#include <cstring> 

#include <stdio.h> 
#include <unistd.h> 
#include <fcntl.h> 
#include <sys/socket.h>
#include <arpa/inet.h> 


int hongX::createListenFd(int port)
{
    // 处理非法端口
    port = ((port <= 1024) || (port >= 65535)) ? 6666 : port;

    // 创建套接字（IPv4，TCP，非阻塞）
    int listenFd = 0;
    if ((listenFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1) {
        LOG_ERROR("[Utils::createListenFd]fd = %d socket : %s\n", listenFd, strerror(errno));
        return n_res_failure;
    }

    // 避免"Address already in use"
    int optval = 1;
    if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int)) == -1) {
        LOG_ERROR("[Utils::createListenFd]fd = %d setsockopt : %s\n", listenFd, strerror(errno));
        return n_res_failure;
    }

    // 绑定IP和端口
    struct sockaddr_in serverAddr;
    ::bzero((char*)&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    serverAddr.sin_port = ::htons((unsigned short)port);
    if (::bind(listenFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        LOG_ERROR("[Utils::createListenFd]fd = %d bind : %s\n", listenFd, strerror(errno));
        return n_res_failure;
    }

    // 开始监听，最大监听队列为LISTENQ
    if (listen(listenFd, g_listen_len) == -1) {
        LOG_ERROR("[Utils::createListenFd]fd = %d listen : %s\n", listenFd, strerror(errno));
        return n_res_failure;
    }

    // 关闭无效监听描述符
    if (listenFd == -1) {
        close(listenFd);
        return n_res_failure;
    }

    return listenFd;
}

int hongX::setNonBlocking(int fd)
{
    // 获取套接字选项
    int flag = ::fcntl(fd, F_GETFL, 0);
    if (flag == -1) {
        printf("[Utils::setNonBlocking]fd = %d fcntl : %s\n", fd, strerror(errno));
        return -1;
    }
    // 设置非阻塞
    flag |= O_NONBLOCK;
    if (::fcntl(fd, F_SETFL, flag) == -1) {
        printf("[Utils::setNonBlocking]fd = %d fcntl : %s\n", fd, strerror(errno));
        return -1;
    }

    return 0;
}

