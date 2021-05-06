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

		// �����ļ�������
		int  operFd(FdOperateType type, int fd, HttpLink* request, int events);
		int  wait(int timeoutMs); // �ȴ��¼�����, ���ػ�Ծ����������
		void handleEvent(int listenFd, std::shared_ptr<ThreadPool>& threadPool, int eventsNum); // �����¼�������


		void setOnConnect(const ConnCallback& cb)       { onConnect_ = cb;    } // ���������ӻص�����
		void setOnDisConnect(const DisConnCallback& cb) { onDisConnect_ = cb; } // ���ùر����ӻص�����
		void setOnRequest(const HandleReqCallback& cb)  { onRequest_ = cb;    } // ���ô�������ص�����
		void setOnResponse(const HandleResCallback& cb) { onResponse_ = cb;   } // ������Ӧ����ص�����

	private:
		using EventList = std::vector<struct epoll_event>;  // epoll������

		int epollFd_;
		EventList events_;
		ConnCallback onConnect_;
		DisConnCallback onDisConnect_;
		HandleReqCallback onRequest_;
		HandleResCallback onResponse_;
	};

}
