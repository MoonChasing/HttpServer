//Author: MoonChasing

#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *mainloop, int threadnum)
    : mainloop_(mainloop),
    threadnum_(threadnum),
    threadlist_(),
    index_(0)
{
    for(int i = 0; i < threadnum_; ++i)
    {
        EventLoopThread *peventloopthread = new EventLoopThread;
        threadlist_.push_back(peventloopthread);
    }
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    for(int i = 0; i < threadnum_; ++i)
    {
        delete threadlist_[i];
    }
    threadlist_.clear();
}

void EventLoopThreadPool::Start()
{
    if(threadnum_ > 0)
    {
        for(int i = 0; i < threadnum_; ++i)
        {
            threadlist_[i]->Start();
        }
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    if(threadnum_ > 0)
    {
        //RR策略
        EventLoop *loop = threadlist_[index_]->getLoop();
        index_ = (index_ + 1) % threadnum_;
        return loop;
    }
    else
    {
        return mainloop_;
    }
}