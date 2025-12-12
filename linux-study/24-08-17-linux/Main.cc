#include "TcpServer.hpp"
#include "Err.hpp"
#include <memory>
#include "Protocol.hpp"

enum
{
    OK,
    DIV_ERR,
    MOD_ERR,
    OPER_ERR
};

void cal(const Request &req, Response &resp)
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
            resp._result = req._x / req._y;
    }
    break;
    case '%':
    {
        if (req._y == 0)
            resp._exitcode = MOD_ERR;
        else
            resp._result = req._x % req._y;
    }
    break;
    default:
        resp._exitcode = OPER_ERR;
        break;
    }
}

void calculate(Connection *conn)
{
    string onePackage;
    while (PartOnepackge(conn->inbuffer_, &onePackage))
    {
        string req_str;
        // 1.2 我们保证，我们req_text里面一定是一个完整的请求："content_len"\r\n"x op y"\r\n
        if (!Delenth(onePackage, &req_str))
            return;

        cout << "去掉报头的正文：\n"
             << req_str << endl;

        // 2. 对请求Request，反序列化
        // 2.1 得到一个结构化的请求对象
        Request req;
        if (!req.deserialize(req_str))
            return;


        // 3. 计算机处理，req.x, req.op, req.y --- 业务逻辑
        // 3.1 得到一个结构化的响应
        Response resp;
        cal(req, resp); // req的处理结果，全部放入到了resp， 回调是不是不回来了？不是！


        // 4.对响应Response，进行序列化
        // 4.1 得到了一个"字符串"
        string resp_str;
        if (!resp.serialize(&resp_str))
            return;

        // 5 构建成为一个完整的报文
        conn->outbuffer_ += Enlenth(resp_str);

        std::cout << "--------------result: " << conn->outbuffer_ << std::endl;
    }
    // 直接发
    if (conn->sender_)
        conn->sender_(conn);
}

static void usage(std::string proc)
{
    std::cerr << "Usage:\n\t" << proc << " port" << "\n\n";
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        usage(argv[0]);
        exit(USAGG_ERR);
    }

    uint16_t port=atoi(argv[1]);
    std::unique_ptr<TcpServer> uls(new TcpServer(calculate,port));
    uls->initServer();
    uls->Dispatch();

    return 0;
}