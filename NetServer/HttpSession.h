// Author: MoonChasing

#ifndef _HTTP_SESSION_H_
#define _HTTP_SESSION_H_

#include <string>
#include <sstream>
#include <map>

typedef struct _HttpRequestContext
{
    std::string method;
    std::string url;
    std::string version;
    std::map<std::string, std::string> header;
    std::string body;
}HttpRequestContext;

typedef struct _HttpResponseContext
{
    std::string version;
    std::string statecode;
    std::string statemsg;
    std::map<std::string, std::string> header;
    std::string body;
}HttpResponseContext;

class HttpSession
{
public:
    HttpSession();
    ~HttpSession();

    void praseHttpRequest(std::string &s);
    void HttpProcess();
    void add2Buf(std::string &s);
    void HttpError(int err_num, std::string short_msg);
    bool keepAlive() {return keepalive_;}
private:
    HttpRequestContext httprequestcontext_;
    std::string responsecontext_;
    std::string responsebody_;
    std::string errormsg;
    std::string path_;
    std::string querystring_;
    bool keepalive_;
    std::string body_buff;    
};
#endif