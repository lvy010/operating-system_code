#include "httpServer.hpp"
#include <memory>

void Usage(string proc)
{
    cout << "\nUsage:\n\t" << proc << " local_port\n\n";
}

string suffixtodes(const string &suff)
{
    string type = "Content-Type: ";
    if (suff == ".html")
        type += "text/html";
    else if (suff == ".jpg")
        type += "application/x-jpg";
    type += "\r\n";
    return type;
}

// 1. 服务器和网页分离，html
// 2. url -> / : web根目录
// 3. 我们要正确的给客户端返回资源类型，我们首先要自己知道！所有的资源都有后缀！！
void Get(const httpRequest &req, httpResponse &resp)
{
    // if(req.path == "test.py")
    // {
    //     //建立进程间通信，pipe
    //     //fork创建子进程，execl("/bin/python", test.py)
    //     // 父进程，将req.parm 通过管道写给某些后端语言，py，java，php等语言
    // }
    // if(req.path == "/search")
    // {
    //     // req.parm
    //     // 使用我们自己写的C++的方法，提供服务
    // }

    cout << "----------------http start---------------" << endl;
    cout << req.inbuffer << endl;
    cout << "method: " << req.method << endl;
    cout << "url: " << req.url << endl;
    cout << "httpversion: " << req.httpversion << endl;
    cout << "path :" << req.path << endl;
    cout << "suffix: " << req.suffix << endl;
    cout << "size: " << req.size << endl;
    cout << "----------------http end-----------------" << endl;

    string respline = "HTTP/1.1 200 OK\r\n";
    //string respline = "HTTP/1.1 307 Temporary Redirect\r\n";
    //string respheader="Content-Type: text/html\r\n";
    string respheader = suffixtodes(req.suffix);
    if (req.size > 0)
    {
        respheader += "Content-Length: ";
        respheader += to_string(req.size);
        respheader += "\r\n";
    }

    //respheader += "Location: https://www.baidu.com/\r\n";

    respheader+="Set-Cookie: 123456abc\r\n";//

    string respblank = "\r\n";

    // string body="<html lang=\"en\"><head><meta charset=\"UTF-8\"><title>for test</title><h1>hello world</h1></head><body><p>北京交通广播《一路畅通》“交通大家谈”节目，特邀北京市交通委员会地面公交运营管理处处长赵震、北京市公安局公安交通管理局秩序处副处长 林志勇、北京交通发展研究院交通规划所所长 刘雪杰为您解答公交车专用道6月1日起社会车辆进出公交车道须注意哪些？</p></body></html>";
    string body;
    body.resize(req.size + 1);
    if (!Util::readFile(req.path, body))
    {
        // 找不到文件,文件大小是-1,要返回404.html,因此重新计算大小
        struct stat sif;
        if (stat(html_404.c_str(), &sif) == 0)
            body.resize(sif.st_size + 1);
        Util::readFile(html_404, body); // 一定能成功
    }

    resp.outbuffer += respline;
    resp.outbuffer += respheader;
    resp.outbuffer += respblank;

    cout << "----------------------http response start---------------------------" << endl;
    cout << resp.outbuffer << endl;
    cout << "----------------------http response end---------------------------" << endl;

    resp.outbuffer += body;
}

// ./httpserver port
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        exit(USAGG_ERR);
    }
    uint16_t serverport = atoi(argv[1]);

    unique_ptr<httpServer> tsv(new httpServer(serverport));
    // httpsvr->registerCb("/", Get); // 功能路由！
    // httpsvr->registerCb("/search", Search);
    // httpsrv->registerCb("/test.py", Other);

    tsv->initServer();
    tsv->start(Get);

    return 0;
}