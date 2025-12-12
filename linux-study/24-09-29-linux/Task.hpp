#pragma once
#include <iostream>
#include <functional>
#include <string>
using namespace std;

class Task
{
    typedef function<int(int, int, char)> func_t;

public:
    Task() {}

    Task(int x, int y, char op, func_t func) : _x(x), _y(y), _op(op), _callback(func)
    {
    }

    // 把任务返回去可以看到
    string operator()()
    {
        int result = _callback(_x, _y, _op);
        char buffer[1024];
        snprintf(buffer, sizeof buffer, "%d %c %d = %d", _x, _op, _y, result);
        return buffer;
    }

    // 把生产的任务也打印出来
    string toTaskString()
    {
        char buffer[1024];
        snprintf(buffer, sizeof buffer, "%d %c %d = ?", _x, _op, _y);
        return buffer;
    }

private:
    int _x;
    int _y;
    char _op;         // 对应+-*/%操作
    func_t _callback; // 回调函数
};

string oper = "+-*/%";

// 回调函数
int mymath(int x, int y, char op)
{
    int result = 0;
    switch (op)
    {
    case '+':
        result = x + y;
        break;
    case '-':
        result = x - y;
        break;
    case '*':
        result = x * y;
        break;
    case '/':
    {
        if (y == 0)
        {
            cout << "div zero error" << endl;
            result = -1;
        }
        else
        {
            result = x / y;
        }
    }
    break;
    case '%':
    {
        if (y == 0)
        {
            cout << "mod zero error" << endl;
            result = -1;
        }
        else
        {
            result = x % y;
        }
    }
    break;
    default:
        break;
    }
    return result;
}

class SaveTask
{
    typedef function<void(string)> func_t;
public:
    SaveTask(){}

    SaveTask(string messages,func_t func):_messages(messages),_func(func)
    {}

    void operator()()
    {
        _func(_messages);
    }

private:
    string _messages;
    func_t _func;
};

void Save(string messages)
{
    string target="./log.txt";
    FILE* fp=fopen(target.c_str(),"a+");
    if(!fp)
    {
        cout<<"fopen error"<<endl;
        return;
    }
    fprintf(fp,"%s\n",messages.c_str());
    fclose(fp);
}