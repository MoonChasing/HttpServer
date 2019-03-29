// Author: MoonChasing

// IO复用流程的抽象，等待事件，处理事件，执行其他任务

#ifndef _EVENTLOOP_H_
#define _EVENTLOOP_H_

#include <functional>
#include <vector>
#include <thread>
#include <mutex>
#include "Poller.h"
#include "Channel.h"

class EventLoop
{
public:
    typedef std::function<void()> Functor;
    typedef std::vector<Channel *> ChannelVec;
    EventLoop();
    ~EventLoop();

    void loop();
    void addChannel2Poller(Channel *pChannel) {poller_.addChannel(pChannel);}
    void removeChannel2Poller(Channel *pChannel) {poller_.removeChannel(pChannel);}
    void updateChannel2Poller(Channel *pChannel) {poller_.updateChannel(pChannel);}

    void Quit() {this->quit = true;}
    std::thread::id getThreadID() {return tid_;}

    void wakeup();
    void handleRead();
    void handleError();

    //Task
    void addTask(Functor fun)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            functorVec_.push_back(fun);
        }
        wakeup();
    }

    void ExecuteTask()
    {
        std::vector<Functor> functorVec;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            functorVec.swap(functorVec_);
        }
        for(Functor &fun : functorVec)
        {
            fun();
        }
    }

private:
    std::vector<Functor> functorVec_;
    ChannelVec channelVec_;
    ChannelVec activeChannelVec_;
    Poller poller_;
    bool quit;
    std::thread::id tid_;
    std::mutex mutex_;
    int wakeupfd_;
    Channel wakeupChannel_;
};

#endif