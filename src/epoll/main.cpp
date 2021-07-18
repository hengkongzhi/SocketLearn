#include "CELLEpoll.hpp"
#include <stdio.h>
#include <vector>
#include <algorithm>
std::vector<SOCKET> g_clients;
bool g_bRun = true;
void cmdThread()
{
    while (true)
    {
        char cmdBuf[256] = {0};
        scanf("%s", cmdBuf);
        if (0 == strcmp(cmdBuf, "exit"))
        {
            g_bRun = false;
            printf("退出cmdThread线程\n");
            break;
        }
        else
        {
            printf("不支持的命令。\n");
        }
    }
}

char g_szBuff[4096] = {};
int  g_nLen = 0;
int readData(SOCKET cSock)
{

    g_nLen = (int)recv(cSock, g_szBuff, 4096, 0);
    return g_nLen;

}
int writeData(SOCKET cSock)
{
    if (g_nLen > 0)
    {
        int nLen = (int)send(cSock, g_szBuff, g_nLen, 0);
        g_nLen = 0;
        return nLen;
    }
    return 1;
    
    

}
int clientLeave(SOCKET cSock)
{
    close(cSock);
    auto itr = std::find(g_clients.begin(), g_clients.end(), cSock);
    g_clients.erase(itr);
    printf("客户端<Socked=%d>已退出.\n", cSock);
}
int main()
{
    std::thread t1(cmdThread);
    t1.detach();
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
    _sin.sin_addr.s_addr = INADDR_ANY;
    if (SOCKET_ERROR == bind(sock, (sockaddr*)&_sin, sizeof(_sin)))
    {
        printf("错误,绑定端口失败...\n");
    }
    else
    {
        printf("绑定端口成功...\n");
    }
    if (SOCKET_ERROR == listen(sock, 100))
    {
        printf("错误,监听网络端口失败...\n");
    }
    else
    {
        printf("监听网络端口成功...\n");
    }

    CELLEpoll ep;
    ep.create(256);
    ep.cellEpollCtl(EPOLL_CTL_ADD, sock, EPOLLIN);
    int msgCount = 0;
    while (g_bRun)
    {
        int n = ep.wait(1);
        auto events = ep.events();
        if (n < 0)
        {
            printf("error,epoll_wait ret=%d\n", n);
            break;
        }
        for (int i = 0; i < n; i++)
        {
            if (events[i].data.fd == sock)
            {
                sockaddr_in clientAddr = {};
                int nAddrLen = sizeof(sockaddr_in);
                SOCKET _cSock = INVALID_SOCKET;
                _cSock = accept(sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
                if (INVALID_SOCKET == _cSock)
                {
                    printf("错误,接受到无效客户端SOCKET...\n");
                }
                else
                {
                    g_clients.push_back(_cSock);
                    ep.cellEpollCtl(EPOLL_CTL_ADD, _cSock, EPOLLIN);
                    printf("新客户端加入：socket = %d,IP = %s\n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
                }
                continue;
            }
            if (events[i].events & EPOLLIN)
            {
                printf("EPOLLIN | %d\n", ++msgCount);
                auto cSockfd = events[i].data.fd;
                int ret = readData(cSockfd);
                if (ret < 0)
                {
                    clientLeave(cSockfd);
                }
                else
                {
                    printf("收到客户端数据:id = %d socket = %d len = %d\n", msgCount, cSockfd, ret);
                }
                ep.cellEpollCtl(EPOLL_CTL_MOD, cSockfd, EPOLLOUT | EPOLLIN);
            }
            if (events[i].events & EPOLLOUT)
            {
                printf("EPOLLOUT | %d\n", msgCount);
                auto cSockfd = events[i].data.fd;
                int ret = writeData(cSockfd);
                if (ret < 0)
                {
                    clientLeave(cSockfd);
                }
                if (msgCount < 5)
                {
                    ep.cellEpollCtl(EPOLL_CTL_MOD, cSockfd, EPOLLIN);
                }
                else
                {
                    ep.cellEpollCtl(EPOLL_CTL_DEL, cSockfd, 0);
                }       
            }
            // if (events[i].events & EPOLLERR)
            // {
            //     printf("EPOLLERR: id = %d socket = %d \n", msgCount, events[i].data.fd);
            // }
            // if (events[i].events & EPOLLHUP)
            // {
            //     printf("EPOLLHUP: id = %d socket = %d \n", msgCount, events[i].data.fd);
            // }
        }
    }
    for (auto client : g_clients)
    {
        close(client);
    }
    close(sock);
    ep.destroy();
    printf("已退出。。。\n");
    return 0;
}