#pragma once

#include<iostream>
#include<string>
#include<stdio.h>
#include <cstdarg>
#include<ctime>
#include<sys/types.h>
#include<unistd.h>
#include<fstream>

#define DEBUG  0
#define NORMAL  1
#define WARNING 2
#define ERROR   3
#define FATAL   4

#define LOG_NORMAL "log.txt"
#define LOG_ERR "log.error"

const char* level_to_string(int level)
{
    switch(level)
    {
        case DEBUG: return "DEBUG";
        case NORMAL: return "NORMAL";
        case WARNING: return "WARNING";
        case ERROR: return "ERROR";
        case FATAL: return "FATAL";
    }
}

//时间戳变成时间
char* timeChange()
{
    time_t now=time(nullptr);
    struct tm* local_time;
    local_time=localtime(&now);

    static char time_str[1024];

    snprintf(time_str,sizeof time_str,"%d-%d-%d %d-%d-%d",local_time->tm_year + 1900,\
                    local_time->tm_mon + 1, local_time->tm_mday,local_time->tm_hour, \
                    local_time->tm_min, local_time->tm_sec);

    return time_str;
}



void logMessage(int level,const char* format,...)
{
    //[日志等级] [时间戳/时间] [pid] [message]
    //[WARNING] [2024-3-21 10-46-03] [123] [创建sock失败]
#define NUM 1024
    //获取时间
    char* nowtime=timeChange();
    char logprefix[NUM];
    snprintf(logprefix,sizeof logprefix,"[%s][%s][pid: %d]",level_to_string(level),nowtime,getpid());

    //
    char logconten[NUM];
    va_list arg;
    va_start(arg,format);
    vsnprintf(logconten,sizeof logconten,format,arg);

    
    std::cout<<logprefix<<logconten<<std::endl;

   
};