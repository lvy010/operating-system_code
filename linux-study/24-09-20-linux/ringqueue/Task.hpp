#pragma once

#include <iostream>
#include <string>
#include <cstdio>
#include <functional>

using namespace std;

class CallTask
{
     typedef function<int(int,int,char)> func_t;
public:
    CallTask()
    {}

    CallTask(int x,int y,char op,func_t func)
        :_x(x)
        ,_y(y)
        ,_op(op)
        ,_func(func)
    {}

    string operator()()
    {
        int result=_func(_x,_y,_op);
        char buffer[1024];
        snprintf(buffer,sizeof buffer,"%d %c %d = %d",_x,_op,_y,result);
        return buffer;
    }

    string toTaskString()
    {
        char buffer[1024];
        snprintf(buffer,sizeof buffer,"%d %c %d = ?",_x,_op,_y);
        return buffer;
    }

private:
    int _x;
    int _y;
    char _op;
    func_t _func;
};


string str="+-*/%";

int mymath(int x,int y,char op)
{
    int result;
    switch(op)
    {
        case '+':
            result=(x+y);
            break;
        case '-':
            result=(x+y);
            break;
        case '*':
            result=(x*y);
            break;
        case '/':
        {
            if(y == 0)
            {
                cout<<"div zero error"<<endl;
                result=-1;
            }
            else
            {
                result=x/y;
            }

        }
            break;
        case '%':
        {
            if(y == 0)
            {
                cout<<"div zero error"<<endl;
                result=-1;
            }
            else
            {
                result=x%y;
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
public:
    typedef function<void(string)> func_t;
    SaveTask()
    {}

    SaveTask(string messages,func_t func)
        :_messages(messages)
        ,_func(func)
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