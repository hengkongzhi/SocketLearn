#ifndef _CELL_EPOLL_HPP_
#define _CELL_EPOLL_HPP_
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/epoll.h>
#include <thread>
#define SOCKET int
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR (-1)
#define EPOLL_ERROR  (-1)
#include "EasyTcpServer.hpp"
class CELLEpoll
{
public:
    ~CELLEpoll()
    {
        destroy();
    }
    int create(int nMaxEvents)
    {
        if (_epfd > 0)
        {
            destroy();
        }
        _epfd = epoll_create(nMaxEvents);
        if (EPOLL_ERROR == _epfd)
        {
           CELLLOG_Info("epoll_create error.");
            return _epfd;
        }
        _pEvents = new epoll_event[nMaxEvents];
        _nMaxEvents = nMaxEvents;
        return _epfd;
    }
    int cellEpollCtl(int op, SOCKET sockfd, uint32_t events)
    {
        epoll_event ev;
        ev.events = events;
        ev.data.fd = sockfd;
        int ret = epoll_ctl(_epfd, op, sockfd, &ev);
        if (ret == EPOLL_ERROR)
        {
            CELLLOG_Info("epoll_ctl1 error.");
        }
        return ret;
    }
    int cellEpollCtl(int op, ClientSocketPtr& pClient, uint32_t events)
    {
        epoll_event ev;
        ev.events = events;
        ev.data.ptr = &pClient;
        int ret = epoll_ctl(_epfd, op, pClient->sockfd(), &ev);
        if (ret == EPOLL_ERROR)
        {
            CELLLOG_Info("epoll_ctl2 error.");
        }
        return ret;
    }
    int wait(int time_out)
    {
        int ret = epoll_wait(_epfd, _pEvents, _nMaxEvents, time_out);
        if (ret == EPOLL_ERROR)
        {
            CELLLOG_Info("epoll_wait error.");
        }
        return ret;
    }
    epoll_event* events()
    {
        return _pEvents;
    }
    void destroy()
    {
        if (_pEvents)
        {
            delete[] _pEvents;
            _pEvents = nullptr;
        }
        if (_epfd > 0)
        {
            close(_epfd);
            _epfd = -1;
        }
    }
private:
    int _epfd = -1;
    int _nMaxEvents = 1;
    epoll_event* _pEvents = nullptr;
};
#endif