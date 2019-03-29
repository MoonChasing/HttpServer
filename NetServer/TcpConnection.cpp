// Author: MoonChasing

#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include "TcpConnection.h"

#define BUFSIZE 4096

int recvn(int fd, std::string &bufferin);
int sendn(int fd, std::string &bufferout);

TcpConnection::TcpConnection(EventLoop *loop, int fd, struct sockaddr_in clientaddr)
    : loop_(loop),
      fd_(fd),
      clientaddr_(clientaddr),
      halfclose_(false),
      disconnected_(false)
{
    //创建事件类，并注册事件执行函数
    pChannel_ = new Channel();
    pChannel_->setfd(fd_);
    pChannel_->setEvents(EPOLLIN | EPOLLOUT);
    pChannel_->setReadHandle(std::bind(&TcpConnection::handleRead, this));
    pChannel_->setWriteHandle(std::bind(&TcpConnection::handleWrite, this));
    pChannel_->setCloseHandle(std::bind(&TcpConnection::handleClose, this));
    pChannel_->setErrorHandle(std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection()
{
    loop_->removeChannel2Poller(pChannel_);
    delete pChannel_;
    close(fd_);
}

void TcpConnection::addChanneltoLoop()
{
    //bug segement fault
    //https://blog.csdn.net/littlefang/article/details/37922113
	//多线程下，加入loop的任务队列
	//主线程直接执行
    //loop_->AddChannelToPoller(pchannel_);
    loop_->addTask(std::bind(&EventLoop::addChannel2Poller, loop_, pChannel_));
}

void TcpConnection::Send(std::string &s)
{
    bufferout_ += s;
    //判断当前线程是不是 Loop IO 线程
    if(loop_->getThreadID() == std::this_thread::get_id())
        sendInLoop();
    else    //不是，则是跨线程调用,加入IO线程的任务队列，唤醒
    {
        loop_->addTask(std::bind(&TcpConnection::sendInLoop, this));
    }
    
}

void TcpConnection::sendInLoop()
{
    int result = sendn(fd_, bufferout_);
    if(result > 0)
    {
        uint32_t events = pChannel_->getEvents();
        if(bufferout_.size() > 0)   //缓冲区满了，数据没发完，就设置EPOLLOUT事件触发
        {
            pChannel_->setEvents(events | EPOLLOUT);
            loop_->updateChannel2Poller(pChannel_);
        }
        else    //数据已发完
        {
            pChannel_->setEvents(events & (~EPOLLOUT));
            sendcompletecallback_(this);

            if(halfclose_)
                handleClose();
        }
        
    }
    else if(result < 0)
    {
        handleError();
    }
    else
    {
        handleClose();
    }
    
}

void TcpConnection::handleRead()
{
    int result = recvn(fd_, bufferin_);

    if(result > 0)
    {
        messagecallback_(this, bufferin_);
    }
    else if(result == 0)
        handleClose();
    else
    {
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    int result = sendn(fd_, bufferout_);
    if(result > 0)
    {
        uint32_t events = pChannel_->getEvents();
        if(bufferout_.size() > 0)
        {
            //缓冲区满了，数据还没发完，设置 EPOLLOUT 事件触发
            pChannel_->setEvents(events | EPOLLOUT);
            loop_->updateChannel2Poller(pChannel_);
        }
        else
        {
            // 数据已发完
            pChannel_->setEvents( events & (~EPOLLOUT) );
            sendcompletecallback_(this);
            // 发送完毕，如果是半关闭状态，则可以close
            if(halfclose_)
                handleClose();
        }
        
    }
}

void TcpConnection::handleError()
{
    if(disconnected_)
        return;
    errorcallback_(this);
    loop_->addTask(connectioncleanup_);
    disconnected_ = true;
}

//对端关闭连接,有两种，一种close，另一种是shutdown(半关闭)，但服务器并不清楚是哪一种，只能按照最保险的方式来，即发完数据再close
void TcpConnection::handleClose()
{
    //优雅关闭， 发完数据再关闭
    if(disconnected_)
        return;
    if(bufferout_.size() > 0 || bufferin_.size() > 0)
    {
        //如果还有数据待发送，则先发完,设置半关闭标志位
		halfclose_ = true;
		//还有数据刚刚才收到，但同时又收到FIN
        if(bufferin_.size() > 0)
            messagecallback_(this, bufferin_);
    }
    else
    {
        loop_->addTask(connectioncleanup_);
        closecallback_(this);
        disconnected_ = true;
    }
    
}

int recvn(int fd, std::string &bufferin)
{
    int nbyte = 0;
    int readsum = 0;
    char buffer[BUFSIZE];
    while(1)
    {
        nbyte = read(fd, buffer, BUFSIZE);

        if(nbyte > 0)
        {
            bufferin.append(buffer, nbyte); //效率较低，两次拷贝
            readsum += nbyte;
            if(nbyte < BUFSIZE)
                return readsum;//读优化，减小一次读调用，因为一次调用耗时10+us
            else
                continue;
        }
        else if(nbyte < 0)
        {
            if (errno == EAGAIN)//系统缓冲区未有数据，非阻塞返回
			{
				//std::cout << "EAGAIN,系统缓冲区未有数据，非阻塞返回" << std::endl;
				return readsum;
			}
			else if (errno == EINTR)
			{
				std::cout << "errno == EINTR" << std::endl;
				continue;
			}
			else
			{
				//可能是RST
				perror("recv error");
				std::cout << "recv error" << std::endl;
				return -1;
			}
        }
        else //返回0，客户端关闭socket，FIN
        {
            std::cout << "Client close the socket." << std::endl;
            return 0;
        }
    }
}

int sendn(int fd, std::string &bufferout)
{
    ssize_t nbyte = 0;
    int sendsum = 0;
    size_t len = 0;

    //零拷贝优化
    len = bufferout.size();
    if(len >= BUFSIZE)
        len = BUFSIZE;

    while(1)
    {
        nbyte = write(fd, bufferout.c_str(), len);
        if(nbyte > 0)
        {
            sendsum += nbyte;
            bufferout.erase(0, nbyte);

            len = bufferout.size();
            if(len >= BUFSIZE)
                len = BUFSIZE;
            if(len == 0)
                return sendsum;
        }
        else if(nbyte < 0) //异常
        {
            if (errno == EAGAIN)//系统缓冲区满，非阻塞返回
			{
				std::cout << "write errno == EAGAIN,not finish!" << std::endl;
				return sendsum;
			}
			else if (errno == EINTR)
			{
				std::cout << "write errno == EINTR" << std::endl;
				continue;
			}
			else if (errno == EPIPE)
			{
				//客户端已经close，并发了RST，继续wirte会报EPIPE，返回0，表示close
				perror("write error");
				std::cout << "write errno == client send RST" << std::endl;
				return -1;
			}
			else
			{
				perror("write error");//Connection reset by peer
				std::cout << "write error, unknow error" << std::endl;
				return -1;
			}
        }
        else 
        {
            //应该不会返回0
            return 0;
        }
    }
}