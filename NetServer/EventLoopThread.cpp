//Author: MoonChasing

#include <iostream>
#include <sstream>
#include "EventLoopThread.h"


EventLoopThread::EventLoopThread()
    : th_(),
    threadid_(-1),
    threadname_("IO thread "),
    loop_(NULL)
{
    
}

EventLoopThread::~EventLoopThread()
{
    //线程结束时清理
    loop_->Quit();
    th_.join();
}

EventLoop* EventLoopThread::getLoop()
{
    return loop_;
}
void EventLoopThread::Start()
{
    //create thread
    th_ = std::thread(&EventLoopThread::ThreadFunc, this);    
}

void EventLoopThread::ThreadFunc()
{
    EventLoop loop;
    loop_ = &loop;

    threadid_ = std::this_thread::get_id();
    std::stringstream sin;
    sin << threadid_;    
    threadname_ += sin.str();

    std::cout << "in the thread:" << threadname_ << std::endl;   
    loop_->loop();
}
