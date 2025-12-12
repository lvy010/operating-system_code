#define _CRT_SECURE_NO_WARNINGS 1
#pragma warning(disable:4996)
#include<iostream>
#include<string>
#include<cstring>
#include<thread>
#include<WinSock2.h>

#pragma comment(lib,"ws2_32.lib")

using namespace std;

//服务器ip,端口
const string serverip = "124.223.54.148";
uint16_t serverport = 8080;

int main()
{
    WSAData wsd;//初始化信息

    //启动Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {/*进行WinSocket的初始化,
        windows 初始化socket网络库，申请2, 2的版本，windows socket编程必须先初始化。*/
        cout << "WSAStartup Error = " << WSAGetLastError() << endl;
        return 0;
    }
    else {
        cout << "WSAStartup Success" << endl;
    }

    //1.创建套接字
    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd  == SOCKET_ERROR) {
        cout << "socket Error = " << WSAGetLastError() << endl;
        return 1;
    }
    else {
        cout << "socket Success" << endl;
    }



    //创建收消息的线程
    thread t1([&]()
        {
            char buffer[1024];
            while (true)
            {
                struct sockaddr_in client;
                int len = sizeof(client);
                int n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&client, &len);
                if (n > 0)
                {
                    buffer[n] = 0;
                    cout << "server reponose: " << buffer << endl;
                }
                else break;

            }
        }
    );

    //发
    struct sockaddr_in peer;
    memset(&peer, 0, sizeof(peer));
    peer.sin_family = AF_INET;
    peer.sin_addr.s_addr = inet_addr(serverip.c_str());
    peer.sin_port = htons(serverport);

    string msg;

    while (true)
    {
        cout << "Please Enter# ";
        getline(cin, msg);
        int n=sendto(sockfd, msg.c_str(), msg.size(), 0, (struct sockaddr*)&peer, sizeof(peer));
        if (n < 0)
        {
            cerr << "sendto error!" << endl;
            break;
        }
    }


    //清理
    closesocket(sockfd);
    WSACleanup();

	return 0;
}