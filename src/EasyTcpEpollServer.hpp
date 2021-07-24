#ifndef _EasyTcpEpollServer_hpp_
#define _EasyTcpEpollServer_hpp_

#include "EasyTcpServer.hpp"
#include "CELLEpollServer.hpp"
#include "CELLEpoll.hpp"

class EasyTcpEpollServer : public EasyTcpServer
{
public:
	void Start(int nCellServer)
	{
		EasyTcpServer::Start<CellEpollServer>(nCellServer); 
	}
protected:
	//处理网络消息
	void OnRun(CELLThread* pThread)
	{
		CELLEpoll ep;
		ep.create(1);
    	ep.cellEpollCtl(EPOLL_CTL_ADD, sockfd(), EPOLLIN);
		// fd_set fdRead;//描述符（socket） 集合
		while (pThread->isRun())
		{
			time4msg();

			int ret = ep.wait(1); //
			if (ret < 0)
			{
				CELLLOG_Info("EasyTcpServer.onRun epoll任务结束。");
				pThread->Exit();
				break;
			}
			auto events = ep.events();
			for (int i = 0; i < ret; i++)
        	{
            	if (events[i].data.fd == sockfd())
            	{
					if (events[i].events & EPOLLIN)
					{
						Accept();
					}
				}
			}
		}
	}
};

#endif // !_EasyTcpServer_hpp_
