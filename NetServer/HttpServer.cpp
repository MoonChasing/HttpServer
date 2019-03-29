//Author: MoonChasing

#include <iostream>
#include <functional>
#include "HttpServer.h"


HttpServer::HttpServer(EventLoop *loop, int port, int iothreadnum, int workerthreadnum)
    : tcpserver_(loop, port, iothreadnum),
    threadpool_(workerthreadnum)
{
    tcpserver_.SetNewConnCallback(std::bind(&HttpServer::HandleNewConnection, this, std::placeholders::_1));
    tcpserver_.SetMessageCallback(std::bind(&HttpServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
    tcpserver_.SetSendCompleteCallback(std::bind(&HttpServer::HandleSendComplete, this, std::placeholders::_1));
    tcpserver_.SetCloseCallback(std::bind(&HttpServer::HandleClose, this, std::placeholders::_1));
    tcpserver_.SetErrorCallback(std::bind(&HttpServer::HandleError, this, std::placeholders::_1));

    threadpool_.start();
}

HttpServer::~HttpServer()
{

}

void HttpServer::HandleNewConnection(TcpConnection *ptcpconn)
{
    //std::string msg(s);
    HttpSession *phttpsession = new HttpSession();
    //可以优化成无锁，放入conn里面就行
    {
        std::lock_guard <std::mutex> lock(mutex_);
        httpsessionnlist_[ptcpconn] = phttpsession;//LOCK
    }
}

void HttpServer::HandleMessage(TcpConnection *ptcpconn, std::string &s)
{ 
    HttpSession *phttpsession = NULL;
    {
        std::lock_guard <std::mutex> lock(mutex_);
        phttpsession =  httpsessionnlist_[ptcpconn];
    }    

    if(threadpool_.getThreadNum() > 0)
    {
        //线程池处理业务，处理完后投递回本IO线程执行send
        //std::cout << "threadpool_.AddTask" << std::endl; 
        threadpool_.addTask([=, &s]() {
            if(s.empty()) return;
            phttpsession->praseHttpRequest(s);
            phttpsession->HttpProcess();
            std::string msg;
            phttpsession->add2Buf(msg);
            ptcpconn->Send(msg);

            if(!phttpsession->keepAlive())
            {
                //短连接，可以告诉框架层数据发完就可以关掉TCP连接，不过这里注释掉，还是交给客户端主动关闭吧
                //ptcpconn->HandleClose();
            }
        });        
    }
    else
    {
        //没有开启业务线程池，业务计算直接在IO线程执行
        phttpsession->praseHttpRequest(s);
        phttpsession->HttpProcess();
        std::string msg;
        phttpsession->add2Buf(msg);
        ptcpconn->Send(msg);

        if(!phttpsession->keepAlive())
        {
            //短连接，可以告诉框架层数据发完就可以关掉TCP连接，不过这里注释掉，还是交给客户端主动关闭吧
            //ptcpconn->HandleClose();
        }        
    }
}

void HttpServer::HandleSendComplete(TcpConnection *ptcpconn)
{

}

void HttpServer::HandleClose(TcpConnection *ptcpconn)
{
    HttpSession *phttpsession = NULL;
    {
        std::lock_guard <std::mutex> lock(mutex_);
        phttpsession = httpsessionnlist_[ptcpconn];
        httpsessionnlist_.erase(ptcpconn);//LOCK
    }
    delete phttpsession;
}

void HttpServer::HandleError(TcpConnection *ptcpconn)
{
    HttpSession *phttpsession = NULL;
    {
        std::lock_guard <std::mutex> lock(mutex_);
        phttpsession = httpsessionnlist_[ptcpconn];
        httpsessionnlist_.erase(ptcpconn);//LOCK
    }
    delete phttpsession;
}

void HttpServer::Start()
{
    tcpserver_.Start();
}
