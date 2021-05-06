#include "HttpServer.h"
using namespace hongX;

// 读取配置文件函数
extern int IniReadStr(const char* fname, const char* section, const char* key, char* value, int size);

int main(int argc, char** argv)
{
	char portStr[10]      = { 0 };
	int nPort			  = 8082;
	char threadNumStr[10] = { 0 };
	int nThreadNum		  = 4;

	if (IniReadStr("server_cfg.ini", "server_config", "port", portStr, 4) == 4)
	{
		printf("read ini get port sucess:%s\n", portStr);
		nPort = atoi(portStr);
	}

	if (IniReadStr("server_cfg.ini", "server_config", "thread_num", threadNumStr, 1) == 1)
	{
		printf("read ini get thread_num sucess:%s\n", threadNumStr);
		nThreadNum = atoi(threadNumStr);
	}

	HttpServer server(nPort, nThreadNum);
	server.run();

	return 0;
}
