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

	const int g_epoll_max_events = 1024; //epoll 监听的描述符上限
	const int g_buffer_size      = 2048; // 缓冲区尺寸
	const int g_connect_timeout  = 5000; // 非活跃连接5秒断开
	const int g_listen_len       = 1024; // 监听队列长度

	// 对描述符的操作类型
	enum FdOperateType
	{
		n_type_operate_fd_add = 0, // 添加
		n_type_operate_fd_mod,	   // 修改
		n_type_operate_fd_del	   // 删除
	};

	/// 返回值及错误码
	enum FunResult
	{
		n_res_sucess    = 0,   // 成功
		n_res_failure   = -1,  // 失败
		n_res_param_err = -2,  // 参数错误
		n_res_nosupport = -3,  // 不支持
		n_res_noexits   = -4,  // 不存在
		n_res_unknow    = -20  // 未知错误
	};

	using ConnCallback		= std::function<void()>;			// 连接事件
	using DisConnCallback   = std::function<void(HttpLink*)>;   // 断开连接事件
	using HandleReqCallback = std::function<void(HttpLink*)>;   // 接收事件
	using HandleResCallback = std::function<void(HttpLink*)>;   // 发送事件

	int createListenFd(int port); // 创建监听描述符
	int setNonBlocking(int fd);   // 设置非阻塞模式


}


