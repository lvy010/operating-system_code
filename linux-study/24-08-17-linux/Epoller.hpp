#pragma once

#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/types.h>
#include "Err.hpp"
#include "Log.hpp"

using namespace std;

const int defaultepfd = -1;
const int size = 128;

class Epoller
{

public:
    Epoller(int epfd = defaultepfd) : _epfd(epfd)
    {
    }

    ~Epoller()
    {
        if (_epfd != defaultepfd)
            close(_epfd);
    }

public:
    void Create()
    {
        _epfd = epoll_create(size);
        if (_epfd < 0)
        {
            logMessage(FATAL, "epoll_create error, code: %d, errstring: %s", errno, strerror(errno));
            exit(EPOLL_CREATE_ERR);
        }
        logMessage(NORMAL, "epoll create success, epfd: %d", _epfd);
    }

    // user -> kernel
    bool AddEvents(int sock, uint32_t event)
    {
        struct epoll_event ev;
        ev.events = event;
        ev.data.fd = sock;
        int n = epoll_ctl(_epfd, EPOLL_CTL_ADD, sock, &ev);
        if (n < 0)
        {
            logMessage(ERROR, "sock join epoll fail");
            return false;
        }
        // logMessage(NORMAL, "sock join epoll success");
        return true;
    }

    // kernel -> user
    int Wait(struct epoll_event *ets, int num, int timeout)
    {
        int n = epoll_wait(_epfd, ets, num, timeout);
        switch (n)
        {
        case 0:
            logMessage(NORMAL, "timeout ...");
            break;
        case -1:
            logMessage(WARNING, "epoll_wait failed, code: %d, errstring: %s", errno, strerror(errno));
            break;
        default:
            logMessage(NORMAL, "have event ready");
            break;
        }
        return n;
    }

    bool Control(int sock, uint32_t event, int action)
    {
        int n = 0;
        if (action == EPOLL_CTL_MOD)
        {
            struct epoll_event ev;
            ev.events = event;
            ev.data.fd = sock;
            n = epoll_ctl(_epfd, action, sock, &ev);
        }
        else if (action == EPOLL_CTL_DEL)
        {
            n = epoll_ctl(_epfd, action, sock, nullptr);
        }
        else
            n = -1;
        return n == 0;
    }

private:
    int _epfd;
};