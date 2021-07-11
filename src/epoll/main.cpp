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
    while (true)
    {

    }

    return 0;
}