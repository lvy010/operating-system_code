#include"udpClient.hpp"
#include<memory>

void Usage(string proc)
{
    cout << "\nUsage:\n\t" << proc << " server_ip server_port\n\n";
}

//./udpClient ip port
int main(int argc,char* argv[])
{
    if(argc != 3)
    {
        Usage(argv[0]);
    }
    string ip=argv[1];
    uint16_t port=atoi(argv[2]);

    unique_ptr<udpClient> ups(new udpClient(ip,port));

    ups->initClient();
    ups->run();

    return 0;
}
