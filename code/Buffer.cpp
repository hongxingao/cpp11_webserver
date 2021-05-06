#include "Buffer.h"

#include <cstring> 
#include <iostream>
#include <unistd.h> 
#include <sys/uio.h> 

using namespace hongX;

Buffer::Buffer()
    : buffer_(g_buffer_size),
      readerIndex_(0),
      writerIndex_(0)
{
    assert(readableBytes() == 0);
    assert(writableBytes() == g_buffer_size);
}

// 从socket读到缓冲区
ssize_t Buffer::readFd(int fd, int* savedErrno)
{
    // 保证一次读到足够多的数据
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = __begin() + writerIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    const ssize_t n = ::readv(fd, vec, 2);
    if(n < 0) 
    {
        LOG_ERROR("[Buffer:readFd]fd = %d readv : %s\n", fd, strerror(errno));
        *savedErrno = errno;
    } 
    else if(static_cast<size_t>(n) <= writable)
        writerIndex_ += n;
    else 
    {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }

    return n;
}

// 缓冲区写到socket
ssize_t Buffer::writeFd(int fd, int* savedErrno)
{
    size_t nLeft = readableBytes();
    char* bufPtr = __begin() + readerIndex_;
    ssize_t n;
    if((n = write(fd, bufPtr, nLeft)) <= 0) 
    {
        if(n < 0 && n == EINTR)
            return 0;
        else 
        {
            LOG_ERROR("[Buffer:writeFd]fd = %d write : %s\n", fd, strerror(errno));
            *savedErrno = errno;
            return -1;
        }
    } 
    else 
    {
        readerIndex_ += n;
        return n;
    }
}

// 取出len个字节 
void Buffer::retrieve(size_t len)
{
    assert(len <= readableBytes());
    readerIndex_ += len;
}

// 取出数据直到end
void Buffer::retrieveUntil(const char* end) 
{
    assert(peek() <= end);
    assert(end <= beginWrite());
    retrieve(end - peek());
}

// 取出buffer内全部数据
void Buffer::retrieveAll() 
{
    readerIndex_ = 0;
    writerIndex_ = 0;
}

// 以string形式取出全部数据
std::string Buffer::retrieveAsString() 
{
    std::string str(peek(), readableBytes());
    retrieveAll();
    return str;
}

// 插入数据
void Buffer::append(const char* data, size_t len) 
{
    ensureWritableBytes(len);
    std::copy(data, data + len, beginWrite());
    hasWritten(len);
}

// 确保缓冲区有足够空间
void Buffer::ensureWritableBytes(size_t len) 
{
    if (writableBytes() < len) {
        __makeSpace(len);
    }
    assert(writableBytes() >= len);
}

// 从头开始查找回车换行
const char* Buffer::findCRLF() const
{
    const char CRLF[] = "\r\n";
    const char* crlf = std::search(peek(), beginWrite(), CRLF, CRLF + 2);
    return crlf == beginWrite() ? nullptr : crlf;
}

// 从start开始查找回车换行
const char* Buffer::findCRLF(const char* start) const
{
    assert(peek() <= start);
    assert(start <= beginWrite());
    const char CRLF[] = "\r\n";
    const char* crlf = std::search(start, beginWrite(), CRLF, CRLF + 2);
    return crlf == beginWrite() ? nullptr : crlf;
}

// 确保缓冲区有足够空间
void Buffer::__makeSpace(size_t len) 
{
    // 缓冲区头部与尾部空闲和
    if (writableBytes() + prependableBytes() < len)
        buffer_.resize(writerIndex_ + len);
    else
    {
        size_t readable = readableBytes();
        std::copy(__begin() + readerIndex_,
            __begin() + writerIndex_,
            __begin());
        readerIndex_ = 0;
        writerIndex_ = readerIndex_ + readable;
        assert(readable == readableBytes());
    }
}