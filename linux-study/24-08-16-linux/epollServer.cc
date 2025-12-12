#include "epollServer.hpp"
#include "err.hpp"
#include <memory>

static void usage(std::string proc)
{
    std::cerr << "Usage:\n\t" << proc << " port" << "\n\n";
}

std::string service(const std::string requst)
{
    return requst;
}


int main(int argc,char* argv[])
{
    if(argc!=2)
    {
        usage(argv[0]);
        exit(USAGG_ERR);
    }

    std::unique_ptr<epollServer> uls(new epollServer(service,atoi(argv[1])));
    uls->initServer();
    uls->start();

    return 0;
}