// Author: MoonChasing

#include <iostream>
#include <sys/eventfd.h>
#include <unistd.h>
#include <stdlib.h>
#include "EventLoop.h"

int createEventfd()
{
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0)
    {
        std::cout << "Failed in eventfd" << std::endl;
        exit(EXIT_FAILURE);
    }
    return evtfd;
}

EventLoop::EventLoop()
    : functorVec_(),
      channelVec_(),
      activeChannelVec_(),
      poller_(),
      quit(true),
      tid_(std::this_thread::get_id()),
      mutex_(),
      wakeupfd_(createEventfd()),
      wakeupChannel_()
{
    wakeupChannel_.setfd(wakeupfd_);
    wakeupChannel_.setEvents(EPOLLIN | EPOLLET);
    wakeupChannel_.setReadHandle(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_.setErrorHandle(std::bind(&EventLoop::handleError, this));
    addChannel2Poller(&wakeupChannel_);
}

EventLoop::~EventLoop()
{
    close(wakeupfd_);
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupfd_, (char *)(&one), sizeof one);
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupfd_, &one, sizeof one);
}

void EventLoop::handleError() {}

void EventLoop::loop()
{
    quit = false;
    while(!quit)
    {
        poller_.poll(activeChannelVec_);
        for(Channel *pChannel : activeChannelVec_)
        {
            pChannel->handleEvent();
        }
        activeChannelVec_.clear();
        ExecuteTask();
    }
}