//Author: MoonChasing
//EventLoopThread类，表示IO线程,执行特定任务的,线程池的是通用任务线程

#ifndef _EVENTLOOP_THREAD_H_
#define _EVENTLOOP_THREAD_H_

#include <iostream>
#include <string>
#include <thread> 
#include "EventLoop.h"

class EventLoopThread
{
private:
    std::thread th_;
    std::thread::id threadid_;
    std::string threadname_;
    EventLoop *loop_;

public:
    EventLoopThread();
    ~EventLoopThread();

    EventLoop* getLoop();
    void Start();
    void ThreadFunc();//线程真正执行的函数
};

#endif