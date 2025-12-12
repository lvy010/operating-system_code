#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include "Util.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

const string sep = "\r\n";
const string default_root = "./wwwroot";
const string home_page = "index.html";
const string html_404 = "./wwwroot/404.html";

class httpRequest
{

public:
    httpRequest(){};
    ~httpRequest(){};

    void parse()
    {
        // 1. 从inbuffer中拿到第一行，分隔符\r\n
        string line = Util::GetOneline(inbuffer, sep);
        if (line.empty())
            return;

        // 2. 从请求行中提取三个字段
        istringstream iss(line);
        iss >> method >> url >> httpversion;

        // 3. 添加web默认路径
        path = default_root + url;
        if (path[path.size() - 1] == '/')
            path += home_page;

        // 3.文件多大
        struct stat sif;
        if(stat(path.c_str(),&sif) == 0)
            size=sif.st_size;
        else    
            size=-1;

        //4. 后缀
        //.htmx   .jpg
        auto pos=path.rfind(".");
        if(pos == string::npos) suffix=".html";
        suffix=path.substr(pos);
    }

public:
    string inbuffer;
    // string reqline;
    // vector<std::string> reqheader;
    // string body;

    string method;
    string url;
    string httpversion;
    string path;
    int size;
    string suffix;
};

class httpResponse
{

public:
    string outbuffer;
};