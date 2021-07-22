#ifndef _CELLEpollServer_hpp_
#define _CELLEpollServer_hpp_

#include "EasyTcpServer.hpp"
#include "CELLEpoll.hpp"

class CellEpollServer : public CellServer
{
public:
	virtual void OnClientJoin(ClientSocketPtr& pClient)
	{
		_ep.cellEpollCtl(EPOLL_CTL_ADD, pClient->sockfd(), EPOLLIN);
	}
	CellEpollServer()
	{
		_ep.create(1300);
	}
	virtual bool DoNetEvents()
	{

		return true;
	}


protected:
	CELLEpoll _ep;

};
#endif
