// Author: MoonChasing

#include<iostream>
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>
#include"Socket.h"

Socket::Socket()
{
    serverfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == serverfd_)
    {
        perror("Server socket create fail!");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server create socket: " << serverfd_ << std::endl; 
}

Socket::~Socket()
{
    close(serverfd_);
    std::cout << "Server close..." << std::endl;
}

void Socket::setSocketOption() {}

void Socket::setReuseAddr()
{
    int on = 1;
    setsockopt(serverfd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
}

void Socket::setNonBlocking()
{
    int opt = fcntl(serverfd_, F_GETFL);
    if(opt < 0)
    {
        perror("fcntl(serverfd_, F_GETFL)");
        exit(EXIT_FAILURE);
    }
    if( fcntl(serverfd_, F_SETFL, opt | O_NONBLOCK) < 0 )
    {
        perror("fcntl setnonblocking");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server setnonblocking..." << std::endl;
}

bool Socket::Bind(int serverport)
{
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(serverport);

    if(-1 == bind(serverfd_, (sockaddr*)&serveraddr, sizeof(serveraddr)))
    {
        close(serverfd_);
        perror("Server bind");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server bind address ok！" << std::endl;
    return true;
}

bool Socket::Listen()
{
    if(-1 == listen(serverfd_, 5))
    {
        perror("Server Listen");
        close(serverfd_);
        exit(EXIT_FAILURE);
    }
    std::cout << "Server listening..." << std::endl;
    return true;
}

int Socket::Accept(struct sockaddr_in &clientaddr)
{
    socklen_t clientaddrlen = sizeof(clientaddr);
    int clientfd = accept(serverfd_, (struct sockaddr *)&clientaddr, &clientaddrlen);
    return clientfd;
}

bool Socket::Close()
{
    close(serverfd_);
    std::cout << "Server close..." << std::endl;
    return true;
}