//Author: MoonChasing

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "TcpServer.h"

void setnonblocking(int fd);

TcpServer::TcpServer(EventLoop* loop, int port, int threadnum)
    : serversocket_(),
    loop_(loop),
    eventloopthreadpool(loop, threadnum),
    serverchannel_(),
    conncount_(0)
{
    serversocket_.setReuseAddr();   
    serversocket_.Bind(port);
    serversocket_.Listen();
    serversocket_.setNonBlocking();

    serverchannel_.setfd(serversocket_.getfd());
    serverchannel_.setReadHandle(std::bind(&TcpServer::OnNewConnection, this));
    serverchannel_.setErrorHandle(std::bind(&TcpServer::OnConnectionError, this));
    
}

TcpServer::~TcpServer()
{

}

void TcpServer::Start()
{
    eventloopthreadpool.Start();

    serverchannel_.setEvents(EPOLLIN | EPOLLET);
    loop_->addChannel2Poller(&serverchannel_);
}

//新TCP连接处理，核心功能，业务功能注册，任务分发
void TcpServer::OnNewConnection()
{
    //循环调用accept，获取所有的建立好连接的客户端fd
    struct sockaddr_in clientaddr;
    int clientfd;
    while( (clientfd = serversocket_.Accept(clientaddr)) > 0) 
    {
        if(++conncount_ >= MAXCONNECTION)
        {
            close(clientfd);
            continue;
        }
        setnonblocking(clientfd);

        //选择IO线程loop
        EventLoop *loop = eventloopthreadpool.getNextLoop();

        //创建连接，注册业务函数
        TcpConnection *ptcpconnection = new TcpConnection(loop, clientfd, clientaddr);
        ptcpconnection->setMessageCallback(messagecallback_);
        ptcpconnection->setSendCompleteCallback(sendcompletecallback_);
        ptcpconnection->setCloseCallback(closecallback_);
        ptcpconnection->setErrorCallback(errorcallback_);
        ptcpconnection->setConnectionCleanup(std::bind(&TcpServer::removeConnection, this, ptcpconnection));

        newconnectioncallback_(ptcpconnection);
        //Bug，应该把事件添加的操作放到最后,否则bug segement fault,导致HandleMessage中的phttpsession==NULL
        //总之就是做好一切准备工作再添加事件到epoll！！！
        ptcpconnection->addChanneltoLoop();
    }
}

//连接清理
void TcpServer::removeConnection(TcpConnection *ptcpconnection)
{
    --conncount_;
    delete ptcpconnection;
}

void TcpServer::OnConnectionError()
{    
    std::cout << "UNKNOWN EVENT" << std::endl;
    serversocket_.Close();
}

void setnonblocking(int fd)
{
    int opts = fcntl(fd, F_GETFL);
    if (opts < 0)
    {
        perror("fcntl(fd,GETFL)");
        exit(1);
    }
    if (fcntl(fd, F_SETFL, opts | O_NONBLOCK) < 0)
    {
        perror("fcntl(fd,SETFL,opts)");
        exit(1);
    }
}
