// Author: MoonChasing

#include <iostream>
#include <stdio.h>
#include <string.h>
#include "HttpSession.h"

HttpSession::HttpSession()
{

}

HttpSession::~HttpSession()
{

}

void HttpSession::praseHttpRequest(std::string &s)
{
    std::string msg;
    msg.swap(s);
    std::string crlf("\r\n"), crlfcrlf("\r\n\r\n");
    size_t prev = 0, next = 0, pos_colon;
    std::string key, value;

    if( (next = msg.find(crlf, prev)) != std::string::npos )
    {
        std::string first_line(msg.substr(prev, next-prev));
        prev = next;
        std::stringstream ss(first_line);
        ss >> httprequestcontext_.method;
        ss >> httprequestcontext_.url;
        ss >> httprequestcontext_.version;
    }
    else
    {
        std::cout << "msg" << msg << std::endl;
        std::cout << "Error in httpPraser: http_request_line isn't complete!" << std::endl;

    }
    
    size_t pos_crlfcrlf = 0;
    if( (pos_crlfcrlf = msg.find(crlfcrlf, prev) ) != std::string::npos )
    {
        while(prev != pos_crlfcrlf)
        {
            next = msg.find(crlf, prev+2);
            pos_colon = msg.find(":", prev+2);
            key = msg.substr(prev+2, pos_colon-prev-2);
            value = msg.substr(pos_colon+2, next-pos_colon-2);
            prev = next;
            httprequestcontext_.header.insert(std::pair<std::string, std::string>(key, value));
        } 
    }
    else
    {
         std::cout << "Error in httpPraser: http_request_header isn't complete!" << std::endl;
    }
    httprequestcontext_.body = msg.substr(pos_crlfcrlf+4);
}

void HttpSession::HttpProcess()
{
    if(httprequestcontext_.method != "GET" && httprequestcontext_.method != "POST")
    {
        std::cout << "HttpServer::HttpParser" << std::endl;
        errormsg = "Method Not Implemented";
        HttpError(501, "Method Not Implemented");
    }
    
    size_t pos = httprequestcontext_.url.find("?");
    if(pos != std::string::npos)
    {
        path_ = httprequestcontext_.url.substr(0, pos);
        querystring_ = httprequestcontext_.url.substr(pos+1);
    }
    else
    {
        path_ = httprequestcontext_.url;
    }
    
    //keepalive判断处理
    std::map<std::string, std::string>::const_iterator it = httprequestcontext_.header.find("Connection");
    if(it != httprequestcontext_.header.end())
    {
        keepalive_ = (it->second == "Keep-Alive");
    }
    else
    {
        if(httprequestcontext_.version == "HTTP/1.1")
        {
            keepalive_ = true;//HTTP/1.1默认长连接
        }
        else
        {
            keepalive_ = false;//HTTP/1.0默认短连接
        }   
    }
    
    responsebody_.clear();
    if(path_ == "/")
        path_ = "/index.html";
    else if("/hello" == path_)
    {
        std::string filetype("text/html");
        responsebody_ = ("hello world");
        responsecontext_ += httprequestcontext_.version + " 200 OK\r\n";
        responsecontext_ += "Server: Chen Shuaihao's NetServer/0.1\r\n";
        responsecontext_ += "Content-Type: " + filetype + "; charset=utf-8\r\n";
        if(it != httprequestcontext_.header.end())
        {
            responsecontext_ += "Connection: " + it->second + "\r\n";
        }
        responsecontext_ += "Content-Length: " + std::to_string(responsebody_.size()) + "\r\n";
        responsecontext_ += "\r\n";
        responsecontext_ += responsebody_;
        return;
    }

    path_.insert(0, ".");
    FILE *fp = fopen(path_.c_str(), "rb");
    if(fp == NULL)
    {
        HttpError(404, "Not Found");
        return;
    }
    else
    {
        char buffer[4096];
        memset(buffer, 0, sizeof(buffer));
        while(1 == fread(buffer, sizeof(buffer), 1, fp))
        {
            responsebody_.append(buffer);
            memset(buffer, 0, sizeof(buffer));
        }
        if(feof(fp))
            responsebody_.append(buffer);
        else
        {
            std::cout << "error fread" << std::endl;
        }
        fclose(fp);
    }
    
    std::string filetype("text/html");
    responsecontext_ += httprequestcontext_.version + " 200 OK\r\n";
    responsecontext_ += "Server: MoonChasing's NetServer/0.1\r\n";
    responsecontext_ += "Content-Type: " + filetype + "; charset=utf-8\r\n";
    if(it != httprequestcontext_.header.end())
    {
        responsecontext_ += "Connection: " + it->second + "\r\n";
    }
    responsecontext_ += "Content-Length: " + std::to_string(responsebody_.size()) + "\r\n";
    responsecontext_ += "\r\n";
    responsecontext_ += responsebody_;    
}

void HttpSession::add2Buf(std::string &s)
{
    responsecontext_.swap(s);
}

void HttpSession::HttpError(int err_num, std::string short_msg)
{
    //这里string创建销毁应该会耗时间
    //std::string body_buff;
    responsebody_ += "<html><title>出错了</title>";
    responsebody_ += "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"></head>";
    responsebody_ += "<style>body{background-color:#f;font-size:14px;}h1{font-size:60px;color:#eeetext-align:center;padding-top:30px;font-weight:normal;}</style>";
    responsebody_ += "<body bgcolor=\"ffffff\"><h1>";
    responsebody_ += std::to_string(err_num) + " " + short_msg;
    responsebody_ += "</h1><hr><em> MoonChasing's NetServer</em>\n</body></html>";

    responsecontext_ += httprequestcontext_.version + " " + std::to_string(err_num) + " " + short_msg + "\r\n";
    responsecontext_ += "Server: MoonChasing's NetServer/0.1\r\n";
    responsecontext_ += "Content-Type: text/html\r\n";
    responsecontext_ += "Connection: Keep-Alive\r\n";
    responsecontext_ += "Content-Length: " + std::to_string(responsebody_.size()) + "\r\n";
    responsecontext_ += "\r\n";
    responsecontext_ += responsebody_;
}