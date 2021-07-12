#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/epoll.h>
#define SOCKET int
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR (-1)
#include <stdio.h>

int main()
{
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
    //linux 2.6.8后size这个值没有意义了
    int epfd = epoll_create(256);
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev);

    //用于接收检测到网络事件的数组
    epoll_event events[256] = {};
    while (true)
    {
        int n = epoll_wait(epfd, events, 256, 0);
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
                    printf("新客户端加入：socket = %d,IP = %s\n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
                }
            }
        }
    }
    close(sock);
    close(epfd);
    printf("已退出。。。\n");
    return 0;
}