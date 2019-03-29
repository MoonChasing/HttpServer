// Author: MoonChasing
// Poller 类： 对 epoll 的封装

#ifndef _POLLER_H_
#define _POLLER_H_

#include<vector>
#include<mutex>
#include<map>
#include<sys/epoll.h>
#include "Channel.h"

class Poller
{
public:
    typedef std::vector<Channel *> ChannelList;

    int pollfd_;
    std::vector<struct epoll_event> eventList_;
    std::map<int, Channel*> channelmap_;
    std::mutex mutex_;

    Poller();
    ~Poller();

    void poll(ChannelList &activeChannelList);
    void addChannel(Channel *pChannel);
    void removeChannel(Channel *pChannel);
    void updateChannel(Channel *pChannel);
};

#endif