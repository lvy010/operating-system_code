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
    ServerERR=2,
    BindERR
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
        _sockfd=socket(AF_INET,SOCK_DGRAM,0);
        if(_sockfd == -1)
        {
            cerr<<"Server fail: "<<strerror(errno)<<endl;
            exit(ServerERR);
        }
        cout << "socket success: " << " : " << _sockfd << endl;

        struct sockaddr_in local;
        bzero(&local,sizeof(local));
        local.sin_family=AF_INET;
        local.sin_port=htons(_port);//网络字节序
        local.sin_addr.s_addr=inet_addr(_ip.c_str());//1.点分十进制->int 2.网络字节序
        //local.sin_addr.s_addr=htonl(INADDR_ANY);//任意地址bind，服务器的真实写法
        
        //2.bind
        int n=bind(_sockfd,(struct sockaddr*)&local,sizeof(local));
        if(n == -1)
        {
            cerr<<"bind fail:"<<strerror(errno)<<endl;
            exit(BindERR);
        }
    }

    void start()
    {
        char buffer[gunm];
        for(;;)
        {
            //读数据
            struct sockaddr_in peer;
            socklen_t len=sizeof(peer);
            ssize_t s=recvfrom(_sockfd,buffer,sizeof(buffer)-1,0,(struct sockaddr*)&peer,&len);

            if(s > 0)
            {
                buffer[s]=0;
                string clientip=inet_ntoa(peer.sin_addr);//1.字节序转变  2.int->点分十进制
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


