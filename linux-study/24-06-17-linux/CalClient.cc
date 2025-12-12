#include"CalClient.hpp"

#include<memory>

void Usage(string proc)
{
    cout << "\nUsage:\n\t" << proc << " local_ip local_port\n\n";
}

// ./tcpClient serverip serverport
int main(int argc,char* argv[])
{
    if(argc != 3)
    {
        Usage(argv[0]);
        exit(1);
    }
    string serverip=argv[1];
    uint16_t serverport=atoi(argv[2]);

    unique_ptr<CalClient> utc(new CalClient(serverip,serverport));
    utc->initClient();
    utc->run();

    return 0;
}