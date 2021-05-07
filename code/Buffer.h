#pragma once

#include <vector>
#include <string>
#include <algorithm> 
#include <iostream>
#include <cassert>
#include "Utils.h"

namespace hongX
{
    // 缓冲区类
    class Buffer {
    public:
        Buffer();
        ~Buffer() { }

        ssize_t      readFd(int fd, int* savedErrno);     // 从socket读到缓冲区
        ssize_t      writeFd(int fd, int* savedErrno);    // 缓冲区写到socket

        // 可读字节数
        size_t       readableBytes() const                 { return writerIndex_ - readerIndex_; }
        // 可写字节数 （缓冲区尾部可写字节数）
        size_t       writableBytes() const                 { return buffer_.size() - writerIndex_; }
        // readerIndex_前面的空闲缓冲区大小
        size_t       prependableBytes() const              { return readerIndex_; }
        // 第一个可读位置
        const char*  peek() const                          { return __begin() + readerIndex_; }

        // 插入数据
        void         append(const std::string& str)        { append(str.data(), str.length());}
        void         append(const void* data, size_t len)  { append(static_cast<const char*>(data), len);}
        // 把其它缓冲区的数据添加到本缓冲区
        void         append(const Buffer& otherBuff)       { append(otherBuff.peek(), otherBuff.readableBytes());}
        // 可写char指针
        char*        beginWrite()                          { return __begin() + writerIndex_; }
        const char*  beginWrite() const                    { return __begin() + writerIndex_; }
        // 写入数据后移动writerIndex_
        void         hasWritten(size_t len)                { writerIndex_ += len; }


        void         retrieve(size_t len);                 // 取出len个字节 
        void         retrieveUntil(const char* end);       // 取出数据直到end
        void         retrieveAll();                        // 取出buffer内全部数据
        void         append(const char* data, size_t len); // 插入数据
        void         ensureWritableBytes(size_t len);      // 确保缓冲区有足够空间
        std::string  retrieveAsString();                   // 以string形式取出全部数据

        // 查找回车换行
        const char* findCRLF() const;
        const char* findCRLF(const char* start) const;

    private:
        // 返回缓冲区头指针
        char* __begin()             { return &*buffer_.begin(); }
        // 返回缓冲区头指针
        const char* __begin() const { return &*buffer_.begin(); }
        
        // 确保缓冲区有足够空间
        void __makeSpace(size_t len);
    
    private:
        std::vector<char> buffer_;             // 缓冲区
        size_t            readerIndex_;        // 读索引
        size_t            writerIndex_;        // 写索引
    }; 

} 
