// Author: MoonChasing

#include<iostream>
#include<map>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<math.h>
#include"Poller.h"

#define MAX_EVENT_NUM 4096
#define TIMEOUT 1000 

Poller::Poller()
    : pollfd_(-1),
      eventList_(MAX_EVENT_NUM),
      channelmap_(),
      mutex_() 
{
    pollfd_ = epoll_create(5);
    if(-1 == pollfd_)
    {
        perror("epoll_create");
        exit(EXIT_FAILURE);
    }
    std::cout << "epoll_create" << pollfd_ << std::endl;
}

Poller::~Poller()
{
    close(pollfd_);
}

//wait I/O event
void Poller::poll(ChannelList &activeChannelList)
{
    int timeout = TIMEOUT;
    int nfds = epoll_wait(pollfd_, &*eventList_.begin(), eventList_.size(), timeout);
    if(nfds == -1)
    {
        printf("errno code is %d", errno);
        perror("epoll_wait");
    }
    for(int i=0; i<nfds; i++)
    {
        int events = eventList_[i].events;

        Channel *pChannel = (Channel *)eventList_[i].data.ptr;
        int fd = pChannel->getfd();

        std::map<int, Channel *>::const_iterator it = channelmap_.find(fd);
        if(it != channelmap_.end())
        {
            pChannel->setEvents(events);
            activeChannelList.push_back(pChannel);
        }
        else
        {
            std::cout << "Not find Channel!" << std::endl;
        }
    }

    if(nfds == eventList_.capacity())
    {
        std::cout << "resize: " << nfds << std::endl;
        eventList_.resize(ceil(nfds * 1.5));
    }
}

void Poller::addChannel(Channel *pChannel)
{
    int fd = pChannel->getfd();
    struct epoll_event event;
    event.events = pChannel->getEvents();
    event.data.ptr = pChannel;
    channelmap_[fd] = pChannel;
    
    if(-1 == epoll_ctl(pollfd_, EPOLL_CTL_ADD, fd, &event))
    {
        perror("epoll add error");
        exit(EXIT_FAILURE);
    }
}

void Poller::removeChannel(Channel *pChannel)
{
    int fd = pChannel->getfd();
    struct epoll_event ev;
    ev.events = pChannel->getEvents();
    ev.data.ptr = pChannel;
    channelmap_.erase(fd);

    if(-1 == epoll_ctl(pollfd_, EPOLL_CTL_DEL, fd, &ev))
    {
        perror("epoll del error");
        exit(EXIT_FAILURE);
    }
}

void Poller::updateChannel(Channel *pChannel)
{
    int fd = pChannel->getfd();
    struct epoll_event ev;
    ev.events = pChannel->getEvents();
    ev.data.ptr = pChannel;

    if(-1 == epoll_ctl(pollfd_, EPOLL_CTL_MOD, fd, &ev))
    {
        perror("epoll update error");
        exit(EXIT_FAILURE);
    }
}