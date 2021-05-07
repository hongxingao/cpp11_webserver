#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <functional>

namespace hongX
{
#define LOG_INFO printf
#define LOG_ERROR printf

	class HttpLink;

	const int g_epoll_max_events = 1024; //epoll ����������������
	const int g_buffer_size      = 2048; // �������ߴ�
	const int g_connect_timeout  = 5000; // �ǻ�Ծ����5��Ͽ�
	const int g_listen_len       = 1024; // �������г���

	// ���������Ĳ�������
	enum FdOperateType
	{
		n_type_operate_fd_add = 0, // ���
		n_type_operate_fd_mod,	   // �޸�
		n_type_operate_fd_del	   // ɾ��
	};

	/// ����ֵ��������
	enum FunResult
	{
		n_res_sucess    = 0,   // �ɹ�
		n_res_failure   = -1,  // ʧ��
		n_res_param_err = -2,  // ��������
		n_res_nosupport = -3,  // ��֧��
		n_res_noexits   = -4,  // ������
		n_res_unknow    = -20  // δ֪����
	};

	using ConnCallback		= std::function<void()>;			// �����¼�
	using DisConnCallback   = std::function<void(HttpLink*)>;   // �Ͽ������¼�
	using HandleReqCallback = std::function<void(HttpLink*)>;   // �����¼�
	using HandleResCallback = std::function<void(HttpLink*)>;   // �����¼�

	int createListenFd(int port); // ��������������
	int setNonBlocking(int fd);   // ���÷�����ģʽ


}


