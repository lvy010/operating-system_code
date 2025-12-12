#include"tcpServer.hpp"
#include<memory>
#include"daemon.hpp"

void Usage(string proc)
{
    cout << "\nUsage:\n\t" << proc << " local_port\n\n";
}

// ./tcpserver port
int main(int argc,char* argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        exit(USAGG_ERR);
    }
    uint16_t serverport=atoi(argv[1]);

    unique_ptr<tcpServer> tsv(new tcpServer(serverport));
    tsv->initServer();

    deamonSelf();
    
    tsv->start();

    return 0;
}