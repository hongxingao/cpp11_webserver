#pragma once

#include "Utils.h"
#include <sys/epoll.h>
#include <vector>
#include <memory>

namespace hongX
{
	class HttpLink;
	class ThreadPool;

	class Epoll
	{
	public:
		Epoll();
		~Epoll();

		// 设置文件描述符
		int  operFd(FdOperateType type, int fd, HttpLink* request, int events);
		int  wait(int timeoutMs); // 等待事件发生, 返回活跃描述符数量
		void handleEvent(int listenFd, std::shared_ptr<ThreadPool>& threadPool, int eventsNum); // 调用事件处理函数


		void setOnConnect(const ConnCallback& cb)       { onConnect_ = cb;    } // 设置新连接回调函数
		void setOnDisConnect(const DisConnCallback& cb) { onDisConnect_ = cb; } // 设置关闭连接回调函数
		void setOnRequest(const HandleReqCallback& cb)  { onRequest_ = cb;    } // 设置处理请求回调函数
		void setOnResponse(const HandleResCallback& cb) { onResponse_ = cb;   } // 设置响应请求回调函数

	private:
		using EventList = std::vector<struct epoll_event>;  // epoll监听的

		int epollFd_;
		EventList events_;
		ConnCallback onConnect_;
		DisConnCallback onDisConnect_;
		HandleReqCallback onRequest_;
		HandleResCallback onResponse_;
	};

}
