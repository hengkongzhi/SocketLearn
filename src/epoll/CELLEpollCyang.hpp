#ifndef _CELL_EPOLL_CYANG_HPP_
#define _CELL_EPOLL_CYANG_HPP_
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/epoll.h>
#include <thread>
#define SOCKET int
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR (-1)
#include <stdio.h>
#include <vector>
#include <algorithm>
class CELLEpollCyang
{
public:
void init(int post)
{
    _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(post);
    _sin.sin_addr.s_addr = INADDR_ANY;
    if (SOCKET_ERROR == bind(_sockfd, (sockaddr*)&_sin, sizeof(_sin)))
    {
        printf("错误,绑定端口失败...\n");
    }
    else
    {
        printf("绑定端口成功...\n");
    }
    if (SOCKET_ERROR == listen(_sockfd, 100))
    {
        printf("错误,监听网络端口失败...\n");
    }
    else
    {
        printf("监听网络端口成功...\n");
    }
    _epfd = epoll_create(256);
    cellEpollCtl(EPOLL_CTL_ADD, _sockfd, EPOLLIN);
}
int clientLeave(SOCKET cSock)
{
    close(cSock);
    auto itr = std::find(_clients.begin(), _clients.end(), cSock);
    _clients.erase(itr);
    printf("客户端<Socked=%d>已退出.\n", cSock);
}
void run()
{
    int n = epoll_wait(_epfd, events, 256, 1);
    if (n < 0)
    {
        printf("error,epoll_wait ret=%d\n", n);
    }
    for (int i = 0; i < n; i++)
    {
        if (events[i].data.fd == _sockfd)
        {
            sockaddr_in clientAddr = {};
            int nAddrLen = sizeof(sockaddr_in);
            SOCKET cSock = INVALID_SOCKET;
            cSock = accept(_sockfd, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
            if (INVALID_SOCKET == cSock)
            {
                printf("错误,接受到无效客户端SOCKET...\n");
            }
            else
            {
                _clients.push_back(cSock);
                cellEpollCtl(EPOLL_CTL_ADD, cSock, EPOLLIN);
                printf("新客户端加入：socket = %d,IP = %s\n", (int)cSock, inet_ntoa(clientAddr.sin_addr));
            }
            continue;
        }

        if (events[i].events & EPOLLIN)
        {
            printf("EPOLLIN | %d\n", ++_msgCount);
            auto cSockfd = events[i].data.fd;
            int ret = readData(cSockfd);
            if (ret < 0)
            {
                clientLeave(cSockfd);
            }
            else
            {
                printf("收到客户端数据:id = %d socket = %d len = %d\n", _msgCount, cSockfd, ret);
            }
            cellEpollCtl(EPOLL_CTL_MOD, cSockfd, EPOLLOUT | EPOLLIN);
        }
        if (events[i].events & EPOLLOUT)
        {
            printf("EPOLLOUT | %d\n", _msgCount);
            auto cSockfd = events[i].data.fd;
            int ret = writeData(cSockfd);
            if (ret < 0)
            {
                clientLeave(cSockfd);
            }
            if (_msgCount < 5)
            {
                cellEpollCtl(EPOLL_CTL_MOD, cSockfd, EPOLLIN);
            }
            else
            {
                cellEpollCtl(EPOLL_CTL_DEL, cSockfd, 0);
            }       
        }
    }
}
int cellEpollCtl(int op, SOCKET sockfd, uint32_t events)
{
    epoll_event ev;
    ev.events = events;
    ev.data.fd = sockfd;
    if (epoll_ctl(_epfd, op, sockfd, &ev) == -1)
    {
        printf("error, epoll_ctl(%d, %d, %d)\n", _epfd, op, sockfd);
        return SOCKET_ERROR;
    }
    return 0;
}

int readData(SOCKET cSock)
{

    _nLen = (int)recv(cSock, _szBuff, 4096, 0);
    return _nLen;

}
int writeData(SOCKET cSock)
{
    if (_nLen > 0)
    {
        int nLen = (int)send(cSock, _szBuff, _nLen, 0);
        _nLen = 0;
        return nLen;
    }
    return 1;
}
void epClose()
{
    for (auto client : _clients)
    {
        ::close(client);
    }
    ::close(_sockfd);
    ::close(_epfd);
}
private:
    int _epfd;
    SOCKET _sockfd;
    char _szBuff[4096] = {};
    int  _nLen = 0;
    //用于接收检测到网络事件的数组
    epoll_event events[256] = {};
    int _msgCount = 0;
    std::vector<SOCKET> _clients;

};
#endif