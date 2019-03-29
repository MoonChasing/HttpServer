//Author: MoonChasing
//
//EventLoopThread类，表示IO线程,执行特定任务的,线程池的是通用任务线程

#ifndef _EVENTLOOP_THREAD_POOL_H_
#define _EVENTLOOP_THREAD_POOL_H_

#include <iostream>
#include <vector>
#include <string>
#include "EventLoop.h"
#include "EventLoopThread.h"

class EventLoopThreadPool
{
private:
    EventLoop *mainloop_;
    int threadnum_;
    std::vector<EventLoopThread*> threadlist_;
    int index_;

public:
    EventLoopThreadPool(EventLoop *mainloop, int threadnum = 0);
    ~EventLoopThreadPool();

    void Start();
    EventLoop* getNextLoop();
};

#endif