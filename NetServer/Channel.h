// Author: MoonChasing
// Channel 类：fd 和事件的封装

#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include<functional>

class Channel
{
public:
    typedef std::function<void()> Callback;

    Channel();
    ~Channel();

    void setfd(int fd) {fd_ = fd;}

    int getfd() {return fd_;}

    void setEvents(uint32_t events) {events_ = events;}

    uint32_t getEvents() {return events_;}

    void handleEvent();

    void setReadHandle(Callback fun) {readhandler_ = fun;}
    void setWriteHandle(Callback fun) {writehandler_ = fun;}
    void setErrorHandle(Callback fun) {errorhandler_ = fun;}
    void setCloseHandle(Callback fun) {closehandler_ = fun;}

private:
    int fd_;
    uint32_t events_; // epoll events

    //事件触发时执行的函数，在 tcpconn 中注册
    Callback readhandler_;
    Callback writehandler_;
    Callback errorhandler_;
    Callback closehandler_;
};
#endif