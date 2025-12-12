#include "pollServer.hpp"
#include "err.hpp"
#include <memory>

static void usage(std::string proc)
{
    std::cerr << "Usage:\n\t" << proc << " port" << "\n\n";
}

string service(string request)
{
    return request;
}


int main(int argc,char* argv[])
{
    if(argc != 2)
    {
        usage(argv[0]);
        exit(USAGG_ERR);
    }


    unique_ptr<pollServer> usl(new pollServer(service,atoi(argv[1])));
    usl->initServer();
    usl->start();
    
    return 0;
}