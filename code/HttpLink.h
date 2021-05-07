#pragma once

#include "Buffer.h"

#include <string>
#include <map>
#include <iostream>

#define STATIC_ROOT "../resources"

namespace hongX 
{
    class Timer;

    // http连接类
    class HttpLink {
    public:
        // 报文解析状态
        enum HttpRequestParseState 
        { 
            ExpectRequestLine, // 期待解析请求行
            ExpectHeaders,     // 期待解析请求头部
            GotAll
        };

        // HTTP方法
        enum Method 
        { 
            Invalid,
            Get,
            Delete
        };

        // HTTP版本
        enum Version 
        {
            Unknown, 
            HTTP10, 
            HTTP11
        };

        HttpLink(int fd);
        ~HttpLink();

        int fd() { return fd_; } // 返回文件描述符

        int read(int* savedErrno);  // 读数据
        int write(int* savedErrno); // 写数据

        // 把待发送数据压进发送缓冲区
        void appendOutBuffer(const Buffer& buf) { outBuff_.append(buf); }
        // 发送缓冲区可读的尺寸
        int writableBytes()                     { return outBuff_.readableBytes(); }

        // 设置、获取定时器
        void   setTimer(Timer* timer) { timer_ = timer; }
        Timer* getTimer()             { return timer_;  }


        bool parseRequest(); // 解析Http报文
        void resetParse();   // 重置解析状态
        bool parseFinish()           { return state_ == GotAll; } // 是否解析完一个报文
        std::string getPath() const  { return path_;            } // 获取路径
        std::string getQuery() const { return query_;           } // 获取参数
        std::string getHeader(const std::string& field) const;
        std::string getMethod() const;
        bool keepAlive() const; // 是否长连接

    private:
        // 解析请求行
        bool __parseRequestLine(const char* begin, const char* end);
        // 设置HTTP方法
        bool __setMethod(const char* begin, const char* end);
        // 设置URL路径
        void __setPath(const char* begin, const char* end);
   
        // 设置URL参数
        void __setQuery(const char* begin, const char* end) { query_.assign(begin, end); }
        // 设置HTTP版本
        void __setVersion(Version version)                  { version_ = version; }
        
        // 增加报文头
        void __addHeader(const char* start, const char* colon, const char* end);

    private:
        // 网络通信相关
        int    fd_;      // 文件描述符
        Buffer inBuff_;  // 读缓冲区
        Buffer outBuff_; // 写缓冲区
        bool   working_; // 若正在工作，则不能被超时事件断开连接

        // 定时器相关
        Timer* timer_;

        // 报文解析相关
        HttpRequestParseState state_;    // 报文解析状态
        Method                method_;   // HTTP方法
        Version               version_;  // HTTP版本
        std::string           path_;     // URL路径
        std::string           query_;    // URL参数
        std::map<std::string, std::string> headers_; // 报文头部
    }; 

} 

