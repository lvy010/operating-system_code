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

using namespace std;

class tcpClient
{
public:
    tcpClient(const string &ip, const uint16_t &port)
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
            string msg;
            while (true)
            {
                // 发
                cout << "Please Enter# ";
                getline(cin, msg);
                write(_sockfd, msg.c_str(), msg.size());

                // 收
                char buffer[1024];
                ssize_t n = read(_sockfd, buffer, sizeof(buffer));
                if (n > 0)
                {
                    buffer[n] = 0;
                    cout <<"server 回显: " <<buffer << endl;
                }
                else
                {
                    break;
                }
            }
        }
    }

    ~tcpClient()
    {
        if(_sockfd >= 0) close(_sockfd);
    }

private:
    string _serverip;
    uint16_t _serverport;
    int _sockfd;
};