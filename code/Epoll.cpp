#include "Epoll.h"
#include "HttpLink.h"
#include "ThreadPool.h"
#include <iostream>
#include <cstring> 
#include <unistd.h> 
#include <cassert>

using namespace hongX;

Epoll::Epoll()
	: epollFd_(epoll_create(g_epoll_max_events)),
	events_(g_epoll_max_events)
{
    assert(epollFd_ >= 0);
}

Epoll::~Epoll()
{
	close(epollFd_);
}

int Epoll::operFd(FdOperateType type, int fd, HttpLink* request, int events)
{
	if (fd < 0 || request == nullptr)
		return n_res_param_err;
	
	int res = n_res_nosupport;
	struct epoll_event event;
	event.data.ptr = (void*)request; 
	event.events = events;

	if (type == n_type_operate_fd_add)
		res = epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event);
	else if (type == n_type_operate_fd_mod)
		res =  epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &event);
	else if (type == n_type_operate_fd_del)
		res = epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &event);

	return res;
}

// �ȴ��¼�����, ���ػ�Ծ����������
int Epoll::wait(int timeoutMs)
{
	int eventsNum = epoll_wait(epollFd_, &*events_.begin(), events_.size(), timeoutMs);
    if (eventsNum < 0) 
		LOG_ERROR("[Epoll::wait] epoll : %s\n", strerror(errno));

	return eventsNum;
}

// ������Ծ�¼�
void Epoll::handleEvent(int listenFd, std::shared_ptr<ThreadPool>& threadPool, int eventsNum)
{
    for (int i = 0; i < eventsNum; ++i) 
    {
        HttpLink* link = (HttpLink*)(events_[i].data.ptr); 
        int fd = link->fd();

        if (fd == listenFd) 
            onConnect_(); //������
        else 
        {
            // �ų������¼�
            if ((events_[i].events & EPOLLERR) ||
                (events_[i].events & EPOLLHUP) ||
                (!events_[i].events & EPOLLIN)) 
            {
                // ������ر�����
                onDisConnect_(link);
            }
            else if (events_[i].events & EPOLLIN) 
            {
                // �ѿɶ�����ѹ���̳߳�
                threadPool->pushJob(std::bind(onRequest_, link));
            }
            else if (events_[i].events & EPOLLOUT) 
            {
                // �ѿ�д����ѹ���̳߳�
                threadPool->pushJob(std::bind(onResponse_, link));
            }
            else 
                LOG_INFO("[Epoll::handleEvent] unexpected event\n");
            
        }
    }

    return;
}