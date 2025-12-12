#include "CalServer.hpp"
#include <memory>

void Usage(string proc)
{
    cout << "\nUsage:\n\t" << proc << " local_port\n\n";
}

// req: 里面一定是我们的处理好的一个完整的请求对象
// resp: 根据req，进行业务处理，填充resp，不用管理任何读取和写入，序列化和反序列化等任何细节
void Cal(const Request &req, Response &resp)
{
    //// req已经有结构化完成的数据啦，你可以直接使用
    resp._exitcode = OK;
    resp._result = OK;

    switch (req._op)
    {
    case '+':
        resp._result = req._x + req._y;
        break;
    case '-':
        resp._result = req._x - req._y;
        break;
    case '*':
        resp._result = req._x * req._y;
        break;
    case '/':
    {
        if (req._y == 0)
            resp._exitcode = DIV_ERR;
        else
            resp._result=req._x/req._y;
    }
    break;
    case '%':
    {
        if (req._y == 0)
            resp._exitcode = MOD_ERR;
        else
            resp._result=req._x%req._y;
    }
    break;
    default:
        resp._exitcode = OPER_ERR;
        break;
    }

}

// ./tcpserver port
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        exit(USAGG_ERR);
    }
    uint16_t serverport = atoi(argv[1]);

    unique_ptr<CalServer> tsv(new CalServer(serverport));
    tsv->initServer();

    tsv->start(Cal);

    return 0;
}