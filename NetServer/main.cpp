//Author: MoonChasing

#include <signal.h>
#include "EventLoop.h"
#include "HttpServer.h"

EventLoop *lp;
static void sighandler1( int sig_no )   
{   
      exit(0);   
}   
static void sighandler2( int sig_no )   
{   
    lp->Quit();
}   

int main(int argc, char *argv[])
{
    signal(SIGUSR1, sighandler1);
    signal(SIGUSR2, sighandler2);
    //signal(SIGINT, sighandler2);
    signal(SIGPIPE, SIG_IGN);  //SIG_IGN,系统函数，忽略信号的处理程序,客户端发送RST包后，服务器还调用write会触发

    int port = 80;
    int iothreadnum = 4;
    int workerthreadnum = 0;
    if(argc == 4)
    {
        port = atoi(argv[1]);
        iothreadnum = atoi(argv[2]);
        workerthreadnum = atoi(argv[3]);
    }   

    EventLoop loop;
    lp = &loop;
    HttpServer httpserver(&loop, port, iothreadnum, workerthreadnum);
    httpserver.Start();
    loop.loop();
}
