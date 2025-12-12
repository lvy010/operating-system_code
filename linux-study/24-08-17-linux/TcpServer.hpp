#pragma once

#include <iostream>
#include <unordered_map>
#include <functional>
#include <cassert>
#include "Sock.hpp"
#include "Err.hpp"
#include "Log.hpp"
#include "Epoller.hpp"
#include "Util.hpp"

const int defaultport = 8080;
const int defaultsock = -1;
const int defaultnum = 128;

class Connection;

using func_t = function<void(Connection *)>;

class Connection
{
public:
    Connection(int sock = defaultport) : sock_(sock)
    {
    }

    ~Connection()
    {
        if (sock_ == defaultsock)
            close(sock_);
    }

    void Rigster(func_t r, func_t s, func_t e)
    {
        recver_ = r;
        sender_ = s;
        excepter_ = e;
    }

public:
    int sock_;         // 这个类对应的套接字是谁
    string inbuffer_;  // 输入缓存区,这里我们暂时没有考虑处理图片,视频等二进制信息,不然stirng就不太合适了
    string outbuffer_; // 输出缓存区 ,你并不能保证你的写事件就绪

    func_t recver_;   // 从sock_读
    func_t sender_;   // 向sock_写
    func_t excepter_; // 处理sock_,IO的时候上面的异常事件

    uint64_t lasttime_;
};

class TcpServer
{

public:
    TcpServer(func_t f, int port = defaultport, int num = defaultnum) : _service(f), _port(port), _revents(nullptr), _num(num)
    {
    }

    ~TcpServer()
    {
        if (_revents)
            delete[] _revents;
    }

    void initServer()
    {
        // 1.创建套接字
        _sock.sock();
        _sock.Bind(_port);
        _sock.Listen();

        // 2.创建epoll模型
        _epoll.Create();

        // 3. 将目前唯一的一个sock，添加到epoller中， 之前需要先将对应的fd设置成为非阻塞
        // Util::SetNonBlock(_sock.Fd());
        // _epoll.AddEvents(_sock.Fd(), EPOLLIN | EPOLLET);
        AddConnection(_sock.Fd(), EPOLLIN | EPOLLET,
                      bind(&TcpServer::Accepter, this, placeholders::_1), nullptr, nullptr);

        // 4.拷贝数组
        _revents = new struct epoll_event[_num];
    }

    // 事件派发
    void Dispatch()
    {
        int timeout = -1000;
        while (true)
        {
            Loop(timeout);

            // 遍历connections_，计算每一个链接的已经有多长时间没有动了
        }
    }

private:
    bool IsConnectionExists(int sock)
    {
        auto pos = _connections.find(sock);
        return pos != _connections.end();
    }

    void Loop(int &timeout)
    {
        int n = _epoll.Wait(_revents, _num, timeout); // 获取已经就绪的事件

        for (int i = 0; i < n; ++i) // 下面一定是就绪的事件
        {
            uint32_t event = _revents[i].events; // 就绪的事件
            int sock = _revents[i].data.fd;      // 就绪事件的fd

            // 将所有的异常问题，全部转化，成为读写问题
            if (event & EPOLLERR)
                event | EPOLLIN | EPOLLOUT;
            if (event & EPOLLHUP)
                event | EPOLLIN | EPOLLOUT;

            // _listensock和普通sock都有自己对应的回调方法,因此对_listensock和普通sock处是一样的,不用区分
            if ((event & EPOLLIN) && IsConnectionExists(sock) && (_connections[sock]->recver_))
                _connections[sock]->recver_(_connections[sock]);
            if ((event & EPOLLOUT) && IsConnectionExists(sock) && (_connections[sock]->sender_))
                _connections[sock]->sender_(_connections[sock]);
        }
    }

    void recver(Connection *conn)
    {
        //conn->lasttime_=time(nullptr);

        while (true)
        {
            char buffer[1024];
            ssize_t s = read(conn->sock_, buffer, sizeof(buffer) - 1);
            if (s > 0)
            {
                buffer[s] = 0;
                conn->inbuffer_ += buffer;
                _service(conn);
            }
            else if (s == 0)
            {
                if (conn->excepter_)
                {
                    conn->excepter_(conn);
                    return;
                }
            }
            else
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                else if (errno == EIDRM)
                    continue;
                else
                {
                    if (conn->excepter_)
                    {
                        conn->excepter_(conn);
                        return;
                    }
                }
            }
        }
    }

    bool EnableReadWrite(int sock, bool reader, bool writer)
    {
        uint32_t event = (reader ? EPOLLIN : 0) | (writer ? EPOLLOUT : 0) | EPOLLET;
        _epoll.Control(sock, event, EPOLL_CTL_MOD);
    }

    void sender(Connection *conn)
    {
        //conn->lasttime_=time(nullptr);
        while (true)
        {
            ssize_t s = send(conn->sock_, conn->outbuffer_.c_str(), conn->outbuffer_.size(), 0);
            if (s > 0)
            {
                conn->outbuffer_.erase(0, s);
                if (conn->outbuffer_.empty())
                    break;
            }
            else
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                else if (errno == EINTR)
                    continue;
                else
                {
                    if (conn->excepter_)
                    {
                        conn->excepter_(conn);
                        return;
                    }
                }
            }
        }
        // 如果没有发送完毕，需要对对应的sock开启对写事件的关系， 如果发完了，我们要关闭对写事件的关心！
        if (!conn->outbuffer_.empty()) // 设置关心该fd读
            EnableReadWrite(conn->sock_, true, true);
        else
            EnableReadWrite(conn->sock_, true, false);
    }

    void excepter(Connection *conn)
    {
        logMessage(DEBUG, "Excepter begin");
        _epoll.Control(conn->sock_, 0, EPOLL_CTL_DEL);
        _connections.erase(conn->sock_);
        close(conn->sock_);

        logMessage(DEBUG, "关闭%d 文件描述符的所有的资源", conn->sock_);

        delete conn;
    }
    void Accepter(Connection *conn)
    {
        while (true)
        {
            string clientip;
            uint16_t clientport;
            int err = 0; // 获取错误码,用来判断是非阻塞了/读取被打断了/还是真错误了
            int sock = _sock.Accept(&clientip, &clientport, &err);
            if (sock > 0)
            {
                // 新的sock套接字添加到AddConnetions
                AddConnection(sock, EPOLLIN | EPOLLET,
                              bind(&TcpServer::recver, this, placeholders::_1),
                              bind(&TcpServer::sender, this, placeholders::_1),
                              bind(&TcpServer::excepter, this, placeholders::_1));

                logMessage(DEBUG, "get a new link, info: [%s:%d]", clientip.c_str(), clientport);
            }
            else
            {
                if (err == EAGAIN || err == EWOULDBLOCK)
                    break;
                else if (err == EINTR)
                    continue;
                else
                    break;
            }
        }
    }

    void AddConnection(int sock, uint32_t events, func_t recver, func_t sender, func_t excepter)
    {
        // 1.设置非阻塞,ET模式fd要非阻塞
        if (events & EPOLLET)
            Util::SetNonBlock(sock);

        // 2. 该sock创建Connection，并初始化，并添加到connections_
        Connection *conn = new Connection(sock);

        // 3. 给对应的sock设置对应的回调方法
        conn->Rigster(recver, sender, excepter);

        // 4. 其次将sock与它要关心的事件"写透式"注册到epoll中，让epoll帮我们关心
        bool r = _epoll.AddEvents(sock, events);
        assert(r);
        (void)r;

        // 5. 将kv添加到connections_
        _connections.insert(make_pair(sock, conn));

        logMessage(DEBUG, "add new sock : %d in epoll and unordered_map", sock);
    }

public:
private:
    uint16_t _port;

    Sock _sock;
    Epoller _epoll;
    unordered_map<int, Connection *> _connections; // 所有链接集合
    struct epoll_event *_revents;
    int _num;

    func_t _service;
};