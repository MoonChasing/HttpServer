// Author: MoonChasing
// 服务器socket类，封装 socket 描述符及相关的初始化操作

#ifndef _SOCKET_H_
#define _SOCKET_H_

#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

class Socket
{
private:
    int serverfd_;

public:
    Socket();
    ~Socket();

    int getfd() {return serverfd_;}
    void setSocketOption();
    void setReuseAddr();
    void setNonBlocking();
    bool Bind(int server_port);
    bool Listen();
    int Accept(struct sockaddr_in &clientaddr);
    bool Close();
};

#endif