#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <jsoncpp/json/json.h>

#define SEP " "
#define SEP_LEN strlen(SEP)
#define LINE_SEP "\r\n"
#define LINE_SEP_LEN strlen(LINE_SEP)

using namespace std;

// "x op y"  -> "content_len"\r\n"x op y"\r\n
string Enlenth(const string &text)
{
    string send_string = to_string(text.size());
    send_string += LINE_SEP;
    send_string += text;
    send_string += LINE_SEP;

    return send_string;
}

//"content_len"\r\n"x op y"\r\n  -> "x op y"
bool Delenth(const string &packge, string *text)
{
    auto pos = packge.find(LINE_SEP);
    if (pos == string::npos)
        return false;
    string text_len_string = packge.substr(0, pos);
    int text_len = stoi(text_len_string);
    *text = packge.substr(pos + LINE_SEP_LEN, text_len);
    return true;
}

class Request
{
public:
    Request() : _x(0), _y(0), _op(0)
    {
    }

    Request(int x, int y, char op) : _x(x), _y(y), _op(op)
    {
    }

    bool serialize(string *out)
    {
#ifdef MYSELF
        // 结构化 -> "x op y"
        *out = "";
        string x_string = to_string(_x);
        string y_string = to_string(_y);

        *out += x_string;
        *out += SEP;
        *out += _op;
        *out += SEP;
        *out += y_string;
#else
        Json::Value root;
        root["first"] = _x;
        root["second"] = _y;
        root["oper"] = _op;

        Json::FastWriter write;
        *out = write.write(root);
#endif
        return true;
    }

    bool deserialize(const string &in)
    {
#ifdef MYSELF
        // "x op y" -> 结构化
        auto left = in.find(SEP);
        auto right = in.rfind(SEP);
        if (left == string::npos || right == string::npos)
            return false;
        if (left == right)
            return false;
        if (right - (left + SEP_LEN) != 1)
            return false;

        string x_string = in.substr(0, left); // [0, 2) [start, end) , start, end - start
        string y_string = in.substr(right + SEP_LEN);

        if (x_string.empty())
            return false;
        if (y_string.empty())
            return false;
        _x = stoi(x_string);
        _y = stoi(y_string);
        _op = in[left + SEP_LEN];
#else
        Json::Value root;
        Json::Reader reader;
        reader.parse(in, root);

        _x = root["first"].asInt();
        _y = root["second"].asInt();
        _op = root["oper"].asInt();

#endif

        return true;
    }

public:
    int _x;
    int _y;
    char _op;
};

class Response
{
public:
    Response() : _exitcode(0), _result(0)
    {
    }

    Response(int exitcode, int result) : _exitcode(exitcode), _result(result)
    {
    }

    bool serialize(string *out)
    {
#ifdef MYSELF
        // 结构化 -> "_exitcode  _result"
        *out = "";
        *out = to_string(_exitcode);
        *out += SEP;
        *out += to_string(_result);
#else
        Json::Value root;
        root["first"] = _exitcode;
        root["second"] = _result;

        Json::FastWriter write;
        *out = write.write(root);
#endif
        return true;
    }

    bool deserialize(const string &in)
    {
#ifdef MYSELF
        //"_exitcode  _result" ->结构化
        auto pos = in.find(SEP);
        if (pos == string::npos)
            return false;

        string ec_string = in.substr(0, pos);
        string res_string = in.substr(pos + SEP_LEN);

        if (ec_string.empty())
            return false;
        if (res_string.empty())
            return false;

        _exitcode = stoi(ec_string);
        _result = stoi(res_string);
#else
        Json::Value root;
        Json::Reader reader;
        reader.parse(in, root);
        _exitcode = root["first"].asInt();
        _result = root["second"].asInt();
#endif
        return true;
    }

public:
    int _exitcode; // 0：计算成功，!0表示计算失败，具体是多少，定好标准
    int _result;   // 计算结果
};

bool PartOnepackge(string &inbuffer, string *text)
{
    //"content_len"/r/n"x op y"/r/n

    auto pos = inbuffer.find(LINE_SEP);
    if (pos == string::npos) // 没读到一个完整报文
        return false;
    //"content_len"/r/n"x op y"/r/n"content_len" >= 一个完整报文长度
    string text_len_string = inbuffer.substr(0, pos);
    int text_len = stoi(text_len_string);
    int total_len = text_len_string.size() + 2 * LINE_SEP_LEN + text_len;

    cout << "处理前#inbuffer: \n"
         << inbuffer << std::endl;

    if (inbuffer.size() < total_len) // 也没有读到一个完整报文
    {
        cout << "你输入的消息，没有严格遵守我们的协议，正在等待后续的内容, continue" << endl;
        return false;
    }

    // 至少有一个完整的报文
    *text = inbuffer.substr(0, total_len); // 读到一个完整报文
    inbuffer.erase(0, total_len);          // inbuffer内部减去这次读到的一个完整的报文

    cout << "处理后#inbuffer:\n " << inbuffer << endl;
    return true;

}