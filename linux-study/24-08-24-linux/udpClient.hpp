#pragma once

#include<iostream>
#include<string>
#include<stdlib.h>
#include<string.h>
#include<cerrno>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include<pthread.h>

using namespace std;


class udpClient
{
public:
    udpClient(const string& ip,const uint16_t& port)
        :_serverip(ip),_serverport(port),_sockfd(-1)
    {}

    void initClient()
    {
        _sockfd=socket(AF_INET,SOCK_DGRAM,0);
        if(_sockfd == -1)
        {
            cerr<<"Client fail: "<<strerror(errno)<<endl;
            exit(1);
        }
        cout << "socket success: " << " : " << _sockfd << endl;
    }

    //读线程
    static void* readMessage(void* args)
    {
        int sockfd=*(static_cast<int*>(args));

        while(true)
        {
            char buffer[1024];
            struct sockaddr_in peer;
            socklen_t len=sizeof(peer);
            ssize_t n=recvfrom(sockfd,buffer,sizeof(buffer)-1,0,(struct sockaddr*)&peer,&len);
            if(n>=0)
            {
                buffer[n]=0;
                cout<<buffer<<endl;
            }
        }

        return nullptr;
    }


    void run()
    {

        //demo3 读
        pthread_create(&_pid,nullptr,readMessage,(void*)&_sockfd);
        pthread_detach(_pid);


        struct sockaddr_in server;
        memset(&server,0,sizeof(server));
        server.sin_family=AF_INET;
        server.sin_addr.s_addr=inet_addr(_serverip.c_str());
        server.sin_port=htons(_serverport);

        string str;
        while(1)
        {
            //demo1
            // cout<<"Please Enter# ";
            // cin>>str;
            // sendto(_sockfd,str.c_str(),str.size(),0,(struct sockaddr*)&server,sizeof(server));


            // char buffer[1024];
            // struct sockaddr_in peer;
            // socklen_t len=sizeof(peer);
            // ssize_t n=recvfrom(_sockfd,buffer,sizeof(buffer)-1,0,(struct sockaddr*)&peer,&len);
            // if(n>0)
            // {
            //     buffer[n]=0;
            //     cout<<"Server provide translate: "<<buffer<<endl;
            // }


            //demo2
            // cout<<"[wdl@VM-28-3-centos 24test_3_16]$ ";
            // string str;
            // getline(cin,str);
            // sendto(_sockfd,str.c_str(),str.size(),0,(struct sockaddr*)&server,sizeof(server));


            // char buffer[1024];
            // struct sockaddr_in peer;
            // socklen_t len=sizeof(peer);
            // ssize_t n=recvfrom(_sockfd,buffer,sizeof(buffer)-1,0,(struct sockaddr*)&peer,&len);
            // if(n>0)
            // {
            //     buffer[n]=0;
            //     cout<<buffer<<endl;
            // }

            //demo3 


            //发
            cerr<<"Enter# ";
            string str;
            getline(cin,str);
            sendto(_sockfd,str.c_str(),str.size(),0,(struct sockaddr*)&server,sizeof(server)); 
        }

    }

    ~udpClient()
    {}

private:
    string _serverip;
    uint16_t _serverport;
    int _sockfd;

    pthread_t _pid;

};
