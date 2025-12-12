#pragma once

#include <iostream>
#include <string>
#include <stdlib.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "protocol.hpp"

using namespace std;

class CalClient
{
public:
    CalClient(const string &ip, const uint16_t &port)
        : _serverip(ip), _serverport(port), _sockfd(-1)
    {
    }

    void initClient()
    {
        // 1.创建socket套接字
        _sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (_sockfd < 0)
        {
            cerr << "socket fail" << endl;
            exit(2);
        }

        // 2.要不要bind,要不要显示bind?  要bind,不需要显示bind
        // 要不是listen 监听? 不需要
        // 要不要accept? 不需要
        // 自己是发送链接的一方
    }

    void run()
    {
        // 2.发起链接
        struct sockaddr_in server;
        memset(&server, 0, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_port = htons(_serverport);
        server.sin_addr.s_addr = inet_addr(_serverip.c_str());

        if (connect(_sockfd, (struct sockaddr *)&server, sizeof(server)) != 0)
        {
            cerr << "socker connect fail" << endl;
        }
        else
        {
            string msg,inbuffer;
            while (true)
            {
                // 发
                cout << "mycal>> ";
                getline(cin, msg);//1+1
                //Request req(10,10,'+');
                Request req=Pare(msg);
                string req_str;
                req.serialize(&req_str);
                string send_string=Enlenth(req_str);
                cout << "sendstring:\n" << send_string << endl;
                send(_sockfd,send_string.c_str(),send_string.size(),0);

                // 收
                string resp_text,resp_str;
                if(!recvpackge(_sockfd,inbuffer,&resp_text))
                    continue;
                if(!Delenth(resp_text,&resp_str))
                    continue;
                
                Response resp;
                resp.deserialize(resp_str);
                cout<<"exitcode: "<<resp._exitcode<<endl;
                cout<<"result: "<<resp._result<<endl;
            }
        }
    }

    Request Pare(const string& msg)
    {
        //1+1
        int pos=0;
        for(int i=0;i<msg.size();++i)
        {
            if(isdigit(msg[i]) == false)
            {
                pos=i;
                break;
            }
        }
        string left=msg.substr(0,pos);
        string right=msg.substr(pos+1);
        Request req;
        req._x=stoi(left);
        req._y=stoi(right);
        req._op=msg[pos];
        return req;
    }

    ~CalClient()
    {
        if(_sockfd >= 0) close(_sockfd);
    }

private:
    string _serverip;
    uint16_t _serverport;
    int _sockfd;
};