#include "udpServer.hpp"
#include <memory>
#include <unordered_map>
#include <fstream>
#include <signal.h>
#include <stdio.h>

void Usage(string proc)
{
    cout << "\nUsage:\n\t" << proc << " local_port\n\n";
}


void handerMessage(int sockfd, string clientip, uint16_t clientport, string message)
{
    string renopose_str;

    // 返回
    struct sockaddr_in client;
    bzero(&client, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(clientip.c_str());
    client.sin_port = htons(clientport);

    sendto(sockfd, renopose_str.c_str(), renopose_str.size(), 0, (struct sockaddr *)&client, sizeof(client));
}


// ./udpServer port
int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        Usage(argv[0]);
    }
    uint16_t port = atoi(argv[1]);
    // string ip=argv[1];

    unique_ptr<udpServer> ups(new udpServer(handerMessage, port));


    ups->initServer();
    ups->start();

    return 0;
}