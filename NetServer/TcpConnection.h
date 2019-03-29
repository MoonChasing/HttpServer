// Author: MoonChasing
// TcpConnection类：表示客户端连接

#ifndef _TCP_CONNECTION_H_
#define _TCP_CONNECTION_H_

#include <functional>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include "Channel.h"
#include "EventLoop.h"

class TcpConnection
{
public:
    typedef std::function<void(TcpConnection*)> Callback;
    typedef std::function<void(TcpConnection *, std::string &)> MessageCallback;
    typedef std::function<void()> TaskCallback;
    
    TcpConnection(EventLoop *loop, int fd, struct sockaddr_in clientaddr);
    ~TcpConnection();

    int getfd() {return fd_;}

    void addChanneltoLoop();
    void Send(std::string &s);

    //当前IO线程发送函数 
    void sendInLoop();
    void handleRead();
    void handleWrite();
    void handleError();
    void handleClose();

    void setMessageCallback(MessageCallback cb) {messagecallback_ = cb;}
    void setSendCompleteCallback(Callback cb) {sendcompletecallback_ = cb;}
    void setCloseCallback(Callback cb) {closecallback_ = cb;}
    void setErrorCallback(Callback cb) {errorcallback_ = cb;}
    void setConnectionCleanup(TaskCallback cb) {connectioncleanup_ = cb;}
     
private:
    EventLoop *loop_;
    Channel *pChannel_;
    int fd_;
    struct sockaddr_in clientaddr_;
    bool halfclose_;    //半关闭标志
    bool disconnected_; //已关闭标志

    //读写缓冲
    std::string bufferin_;
    std::string bufferout_;

    MessageCallback messagecallback_;
    Callback sendcompletecallback_;
    Callback closecallback_;
    Callback errorcallback_;
    TaskCallback connectioncleanup_;
};

#endif
