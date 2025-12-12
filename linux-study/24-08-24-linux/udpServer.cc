#include "udpServer.hpp"
#include "onlineUser.hpp"

#include <memory>
#include <unordered_map>
#include <fstream>
#include <signal.h>
#include <stdio.h>

void Usage(string proc)
{
    cout << "\nUsage:\n\t" << proc << " local_port\n\n";
}

const string filename = "dict.txt";
unordered_map<string, string> dict;

bool CurString(string &str, string *s1, string *s2, string step)
{
    auto pos = str.find(step.c_str());
    if (pos != string::npos)
    {
        *s1 = str.substr(0, pos);  //[)
        *s2 = str.substr(pos + 1); //[)
        return true;
    }
    else
    {
        return false;
    }
}

void initDict()
{
    ifstream iss(filename);
    string str, Key, Val;
    while (iss >> str)
    {
        // 分割字符串
        if (CurString(str, &Key, &Val, ":"))
        {
            dict.insert(make_pair(Key, Val));
        }
    }

    cout << "load dict success" << endl;
}

void print()
{
    for (auto &s : dict)
    {
        cout << s.first << ": " << s.second << endl;
    }
}

// void reload(int signo)
// {
//     (void)signo;
//     initDict();
// }

void reload(int signo)
{
    (void)signo;
    initDict();
}

// demo1  翻译
void handerMessage(int sockfd, string clientip, uint16_t clientport, string message)
{
    string renopose_str;
    auto it = dict.find(message);
    if (it != dict.end())
    {
        renopose_str += it->second;
    }
    else
    {
        renopose_str = "UnKnow";
    }

    // 返回
    struct sockaddr_in client;
    bzero(&client, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(clientip.c_str());
    client.sin_port = htons(clientport);

    sendto(sockfd, renopose_str.c_str(), renopose_str.size(), 0, (struct sockaddr *)&client, sizeof(client));
}

// demo2  命令
void execCommand(int sockfd, string clientip, uint16_t clientport, string cmd)
{
    if (cmd.find("rm") != string::npos || cmd.find("mv") != string::npos || cmd.find("rmdir") != string::npos)
    {
        cerr << clientip << ":" << clientport << " 正在做一个非法的操作: " << cmd << endl;
        return;
    }

    string response;
    FILE *pf = popen(cmd.c_str(), "r");
    if (pf == NULL)
        response = cmd + "excel fail";
    char line[1024];
    while (fgets(line, sizeof(line) - 1, pf))
    {
        response += line;
    }
    
    pclose(pf);

    struct sockaddr_in client;
    bzero(&client, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(clientip.c_str());
    client.sin_port = htons(clientport);

    sendto(sockfd, response.c_str(), response.size(), 0, (struct sockaddr *)&client, sizeof(client));

}

static onlineUser onlineuser;

//demo3 聊天室
void routeMessage(int sockfd, string clientip, uint16_t clientport, string message)
{
    if(message == "online") onlineuser.addUser(clientip,clientport);

    if(message == "offline") onlineuser.eraseUser(clientip,clientport);


    if(onlineuser.isOnline(clientip,clientport))
    {
        onlineuser.boradcast(sockfd,clientip,clientport,message);
    }
    else
    {
        struct sockaddr_in client;
        bzero(&client, sizeof(client));
        client.sin_family = AF_INET;
        client.sin_addr.s_addr = inet_addr(clientip.c_str());
        client.sin_port = htons(clientport);
        string s="你还没有上线，请先上线，运行: online";

        sendto(sockfd, s.c_str(), s.size(), 0, (struct sockaddr *)&client, sizeof(client));
    }

}


// ./udpServer port
int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        Usage(argv[0]);
        exit(USAGE_ERR);
    }
    uint16_t port = atoi(argv[1]);
    // string ip=argv[1];

    // demo1 翻译
    //  //热加载
    //  signal(3,reload);

    // initDict();

    // //print();
    // unique_ptr<udpServer> ups(new udpServer(handerMessage,port));

    // demo2 命令
    //unique_ptr<udpServer> ups(new udpServer(execCommand, port));


    //demo3 聊天室
    unique_ptr<udpServer> ups(new udpServer(routeMessage, port));


    ups->initServer();
    ups->start();

    return 0;
}