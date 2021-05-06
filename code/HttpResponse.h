#pragma once

#include <map>
#include <string>

namespace hongX 
{
    class Buffer;

    class HttpResponse 
    {
    public:
        static const std::map<int, std::string> statusCode2Message;
        static const std::map<std::string, std::string> suffix2Type;

        HttpResponse(int statusCode, std::string path, bool keepAlive)
            : statusCode_(statusCode),
              path_(path),
              keepAlive_(keepAlive)
        {}

        ~HttpResponse() {}

        // 构造回应数据 把数据压进缓冲区 返回缓冲区
        Buffer makeResponse();

        void doErrorResponse(Buffer& output, std::string message);
        void doStaticRequest(Buffer& output, long fileSize);

    private:
        std::string __getFileType();

    private:
        std::map<std::string, std::string> headers_; // 响应报文头部
        int statusCode_;                             // 响应状态码
        std::string path_;                           // 请求资源路径
        bool keepAlive_;                              // 长连接

    }; 

} 
