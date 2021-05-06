#include "HttpLink.h"

#include <iostream>
#include <cassert>

#include <unistd.h>

using namespace hongX;

HttpLink::HttpLink(int fd)
    : fd_(fd),
      working_(false),
      timer_(nullptr),
      state_(ExpectRequestLine),
      method_(Invalid),
      version_(Unknown)
{
    assert(fd_ >= 0);
}

HttpLink::~HttpLink()
{
    close(fd_);
}

int HttpLink::read(int* savedErrno)
{
    int ret = inBuff_.readFd(fd_, savedErrno);
    return ret;
}

int HttpLink::write(int* savedErrno)
{
    int ret = outBuff_.writeFd(fd_, savedErrno);
    return ret;
}

bool HttpLink::parseRequest()
{
    bool ok = true;
    bool hasMore = true;

    while(hasMore) 
    {
        if(state_ == ExpectRequestLine) 
        {
            // 处理请求行
            const char* crlf = inBuff_.findCRLF();
            if(crlf) 
            {
                ok = __parseRequestLine(inBuff_.peek(), crlf);
                if(ok) 
                {
                    inBuff_.retrieveUntil(crlf + 2);
                    state_ = ExpectHeaders;
                } 
                else 
                    hasMore = false;
            } 
            else 
                hasMore = false;
            
        } 
        else if(state_ == ExpectHeaders) 
        {
            // 处理报文头
            const char* crlf = inBuff_.findCRLF();
            if(crlf) 
            {
                const char* colon = std::find(inBuff_.peek(), crlf, ':');
                if(colon != crlf) 
                    __addHeader(inBuff_.peek(), colon, crlf);
                else 
                {
                    state_ = GotAll;
                    hasMore = false;
                }
                inBuff_.retrieveUntil(crlf + 2);
            }
            else 
                hasMore = false;
        } 
    }

    return ok;
}

bool HttpLink::__parseRequestLine(const char* begin, const char* end)
{
    bool succeed = false;
    const char* start = begin;
    const char* space = std::find(start, end, ' ');
    if(space != end && __setMethod(start, space)) 
    {
        start = space + 1;
        space = std::find(start, end, ' ');
        if(space != end) 
        {
            const char* question = std::find(start, space, '?');
            if(question != space) 
            {
                __setPath(start, question);
                __setQuery(question, space);
            } 
            else 
                __setPath(start, space);
            
            start = space + 1;
            succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
            if(succeed) 
            {
                if(*(end - 1) == '1')
                    __setVersion(HTTP11);
                else if(*(end - 1) == '0')
                    __setVersion(HTTP10);
                else
                    succeed = false;
            } 
        }
    }

    return succeed;
}

bool HttpLink::__setMethod(const char* start, const char* end)
{
    std::string m(start, end);
    if(m == "GET")
        method_ = Get;
    else if(m == "DELETE")
        method_ = Delete;
    else
        method_ = Invalid;

    return method_ != Invalid;
}

void HttpLink::__addHeader(const char* start, const char* colon, const char* end)
{
    std::string field(start, colon);
    ++colon;
    while(colon < end && *colon == ' ')
        ++colon;
    std::string value(colon, end);
    while(!value.empty() && value[value.size() - 1] == ' ')
        value.resize(value.size() - 1);

    headers_[field] = value;
}

std::string HttpLink::getMethod() const
{
    std::string res;
    if(method_ == Get)
        res = "GET";
    else if(method_ == Delete)
        res = "DELETE";
    
    return res;
}

std::string HttpLink::getHeader(const std::string& field) const
{
    std::string res;
    auto itr = headers_.find(field);
    if(itr != headers_.end())
        res = itr->second;
    return res;
}

bool HttpLink::keepAlive() const
{
    std::string connection = getHeader("Connection");
    bool res = connection == "Keep-Alive" || 
               (version_ == HTTP11 && connection != "close");

    return res;
}

void HttpLink::resetParse()
{
    state_   = ExpectRequestLine; // 报文解析状态
    method_  = Invalid;           // HTTP方法
    version_ = Unknown;           // HTTP版本
    path_    = "";                // URL路径
    query_   = "";                // URL参数
    headers_.clear();             // 报文头部
}

// 设置URL路径
void HttpLink::__setPath(const char* begin, const char* end)
{
    std::string subPath;
    subPath.assign(begin, end);
    if (subPath == "/")
        subPath = "/index.html";
    path_ = STATIC_ROOT + subPath;
}