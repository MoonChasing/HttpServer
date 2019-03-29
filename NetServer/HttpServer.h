// Author: MoonChasing

#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include <string>
#include <mutex>
#include <map>
#include "EventLoop.h"
#include "TcpServer.h"
#include "TcpConnection.h"
#include "HttpSession.h"
#include "ThreadPool.h"

class HttpServer
{
private:
    void HandleNewConnection(TcpConnection *ptcpconn);
    void HandleMessage(TcpConnection *ptcpconn, std::string &s);
    void HandleSendComplete(TcpConnection *ptcpconn);
    void HandleClose(TcpConnection *ptcpconn);
    void HandleError(TcpConnection *ptcpconn);

    TcpServer tcpserver_;
    ThreadPool threadpool_;
    std::map<TcpConnection *, HttpSession*> httpsessionnlist_;//管理Http会话
    std::mutex mutex_;

public:
    HttpServer(EventLoop *loop, int port, int iothreadnum, int workerthreadnum);
    ~HttpServer();

    void Start();

};

#endif