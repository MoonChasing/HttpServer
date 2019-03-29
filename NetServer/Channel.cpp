// Author: MoonChasing
// Channel 类：表示每一个客户端连接的通道

#include <iostream>
#include <sys/epoll.h>
#include "Channel.h"

Channel::Channel() {}

Channel::~Channel() {}

void Channel::handleEvent()
{
    if(events_ & EPOLLRDHUP)
    {
        std::cout << "Event EPOLLRDHUP" << std::endl;
        closehandler_();
    }
    else if( events_ & (EPOLLIN | EPOLLPRI) )
    {
        readhandler_();
    }
    else if(events_ & EPOLLOUT)
    {
        //std::cout<< "Event EPOLLOUT" << std::endl;
        writehandler_();
    }
    else
    {
        std::cout << "Something else happened." << std::endl;
        errorhandler_();
    }
    
}