#ifndef _EasyTcpSelectServer_hpp_
#define _EasyTcpSelectServer_hpp_

#include "EasyTcpServer.hpp"
#include "CELLSelectServer.hpp"
#include "CELLFDSet.hpp"

class EasyTcpSelectServer : public EasyTcpServer
{
public:
	void Start(int nCellServer)
	{
		EasyTcpServer::Start<CellSelectServer>(nCellServer); 
	}
protected:
	//处理网络消息
	void OnRun(CELLThread* pThread)
	{
		CELLFDSet fdRead;
		// fd_set fdRead;//描述符（socket） 集合
		while (pThread->isRun())
		{
			time4msg();
			//伯克利套接字 BSD socket
			
			//清理集合
			fdRead.zero();
			// FD_ZERO(&fdRead);
			//将描述符（socket）加入集合
			fdRead.add(sockfd());
			// FD_SET(sockfd(), &fdRead);
			///nfds 是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
			///既是所有文件描述符最大值+1 在Windows中这个参数可以写0
			timeval t = { 0,1};
			int ret = select(sockfd() + 1, fdRead.fdset(), 0, 0, &t); //
			if (ret < 0)
			{
				CELLLOG_Info("EasyTcpServer.onRun Select任务结束。");
				pThread->Exit();
				break;
			}
			//判断描述符（socket）是否在集合中
			if (fdRead.has(sockfd()))
			{
				fdRead.del(sockfd());
				// FD_CLR(sockfd(), &fdRead);
				Accept();
			}
		}
	}
};

#endif // !_EasyTcpServer_hpp_
