// Author: MoonChasing

#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include <functional>
#include <string>
#include <map>
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "TcpConnection.h"
#include "EventLoopThreadPool.h"

#define MAXCONNECTION 20000

class TcpServer
{
public:
    typedef std::function<void(TcpConnection*, std::string&)> MessageCallback;
    typedef std::function<void(TcpConnection*)> Callback;
    TcpServer(EventLoop* loop, int port, int threadnum = 0);
    ~TcpServer();

    void Start();

    //业务函数注册
    void SetNewConnCallback(Callback cb)
    {
        newconnectioncallback_ = cb; 
    }
    void SetMessageCallback(MessageCallback cb)
    {
        messagecallback_ = cb;
    }
    void SetSendCompleteCallback(Callback cb)
    { 
        sendcompletecallback_ = cb; 
    }
    void SetCloseCallback(Callback cb)
    {
        closecallback_ = cb;
    }    
    void SetErrorCallback(Callback cb)
    { 
        errorcallback_ = cb;
    }


private:
    /* data */
    Socket serversocket_;
    EventLoop *loop_;
    EventLoopThreadPool eventloopthreadpool;
    Channel serverchannel_;
    int conncount_;
    std::map<int, TcpConnection*> tcpconnlist_;

    //业务接口函数
    Callback newconnectioncallback_;
    MessageCallback messagecallback_;
    Callback sendcompletecallback_;
    Callback closecallback_;
    Callback errorcallback_;

    //服务器对连接处理的函数，业务无关
    void OnNewConnection();
    void removeConnection(TcpConnection *ptcpconnection);
    void OnConnectionError();

};


#endif