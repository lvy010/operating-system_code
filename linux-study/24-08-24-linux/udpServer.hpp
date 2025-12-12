#pragma once

#include<iostream>
#include<string>
#include<stdlib.h>
#include<string.h>
#include<cerrno>
#include<functional>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace std;

static const string defaultIP="0.0.0.0";
const int gunm=1024;

enum
{
    USAGE_ERR,
    SOCKET_ERR=2,
    BIND_ERR
};


class udpServer
{ 
    typedef function<void(int,string,uint16_t,string)> func_t;

public:
    udpServer(const func_t& func,const uint16_t& port,const string& ip=defaultIP)
        :_callback(func),_port(port),_ip(ip),_sockfd(-1)
    {}

    void initServer()
    {
        //1.创建socket
        _sockfd=socket(AF_INET,SOCK_DGRAM,0);//这里 AF_INET,是创建一个网络通信套接字
        if(_sockfd == -1)
        {
            cerr<<"Server fail: "<<strerror(errno)<<endl;
            exit(SOCKET_ERR);
        }
        cout << "socket success: " << " : " << _sockfd << endl;

        //2.bind，绑定ip+port
        struct sockaddr_in local;//定义了一个变量,在栈上,用户层
        bzero(&local,sizeof(local));//结构体内容初始设置为0
        local.sin_family=AF_INET;//采用网络通信, AF_INET填充一个struct sockaddr_in结构体,为了用于网络通信
        local.sin_port=htons(_port);//转成网络字节序, 给别人发消息,也要把自己的port和ip发送给对方
        local.sin_addr.s_addr=inet_addr(_ip.c_str());//1.string->uint32_t 2.htonl
        //local.sin_addr.s_addr=htonl(INADDR_ANY);//任意地址bind，服务器的真实写法
        
        
        int n=bind(_sockfd,(struct sockaddr*)&local,sizeof(local));
        if(n == -1)
        {
            cerr<<"bind fail:"<<strerror(errno)<<endl;
            exit(BIND_ERR);
        }
    }

    void start()
    {
        //服务器的本质其实就是一个死循环  ---> 常驻内存的进程
        char buffer[gunm];
        for(;;)
        {
            //读数据
            struct sockaddr_in peer;
            socklen_t len=sizeof(peer);//必填
            ssize_t s=recvfrom(_sockfd,buffer,sizeof(buffer)-1,0,(struct sockaddr*)&peer,&len);
            //1. 数据是什么  2.谁发的
            if(s > 0)
            {
                buffer[s]=0;
                string clientip=inet_ntoa(peer.sin_addr);//1.网络字节序 2.int->点分十进制
                uint16_t clientport=ntohs(peer.sin_port);
                string str=buffer;

                cout<<clientip<<"["<<clientport<<"]# "<<str<<endl;

                //并不是收到数据打印就完了,还要就行业务处理
                _callback(_sockfd,clientip,clientport,str);
            }
        }
    }

    ~udpServer()
    {}


private:
    string _ip;
    uint16_t _port;
    int _sockfd;

    func_t _callback;//回调函数
};


