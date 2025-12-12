#pragma once
#include <iostream>
#include <functional>
#include <string>
#include <unistd.h>
#include "logMessage.hpp"
using namespace std;

void serverIO(int sock)
{
    char buffer[1024];
    while (true)
    {
        // 读
        ssize_t n = read(sock, buffer, sizeof(buffer));
        if (n > 0)
        {
            buffer[n] = 0;
            cout << "recv message: " << buffer << endl;

            // 写
            string outbuffer = buffer;
            outbuffer += " server[respond]";
            write(sock, outbuffer.c_str(), outbuffer.size());
        }
        else if (n == 0)
        {
            // 代表clien退出
            logMessage(NORMAL, "client quit, me to!");
            break;
        }
    }
    close(sock);
}

class Task
{
    typedef std::function<void(int)> func_t;

public:
    Task(){};

    Task(int sock, func_t func) : _sock(sock), _callback(func)
    {
    }

    void operator()()
    {
        _callback(_sock);
    }

private:
    int _sock;
    func_t _callback;
};
