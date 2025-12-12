#pragma once
#include<iostream>
#include<functional>
#include<string>

class Task
{
    typedef std::function<int(int,int,char)> func_t;
public:
    Task(){};

    Task(int x,int y,char op,func_t func):_x(x),_y(y),_op(op),_func(func)
    {}

    std::string operator()()
    {
        int result=_func(_x,_y,_op);
        char buffer[64];
        snprintf(buffer,sizeof buffer,"%d %c %d = %d",_x,_op,_y,result);
        return buffer;
    }

    std::string toTaskString()
    {
        char buffer[64];
        snprintf(buffer,sizeof buffer,"%d %c %d = ?",_x,_op,_y);
        return buffer;
    }

private:
    int _x;
    int _y;
    char _op;
    func_t _func;
};

int mymath(int x, int y, char op)
{
    int result = 0;
    switch (op)
    {
    case '+':
        result=x+y;
        break;
    case '-':
        result=x-y;
        break;
    case '*':
        result=x*y;
        break;
    case '/':
        {
            if(y == 0)
            {
                std::cout<<"div zero error"<<std::endl;
                result=-1;
            }
            else
                result=x/y;
        }
        break;
    case '%':
            if(y == 0)
            {
                std::cout<<"mod zero error"<<std::endl;
                result=-1;
            }
            else
                result=x%y;
        break;
    default:
        break;
    }
    return result;
}