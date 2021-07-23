#ifndef _CELLEpollServer_hpp_
#define _CELLEpollServer_hpp_

#include "EasyTcpServer.hpp"
#include "CELLEpoll.hpp"

class CellEpollServer : public CellServer
{
public:
	virtual void OnClientJoin(ClientSocketPtr& pClient)
	{
		_ep.cellEpollCtl(EPOLL_CTL_ADD, pClient, EPOLLIN);
	}
	CellEpollServer()
	{
		_ep.create(1300);
	}
	virtual bool DoNetEvents()
	{
		int ret = _ep.wait(1);
		if (ret < 0)
		{
			CELLLOG_Error("CellEpollServer.wait error.");
			return false;
		}
		else if (ret == 0)
		{
			return true;
		}
		auto events = _ep.events();
		for (int i = 0; i < ret; i++)
        {
			ClientSocketPtr pclient = *(ClientSocketPtr*)events[i].data.ptr;
        	if (pclient.get())
        	{
				if (events[i].events & EPOLLIN)
				{
					if (-1 == RecvData(pclient))
					{
						rmClient(pclient);
						continue;
					}
				}
				if (events[i].events & EPOLLOUT)
				{
					if (-1 ==  pclient->SendDataReal())
					{
						rmClient(pclient);
					}
				}
			}
		}
		return true;
	}
	void rmClient(ClientSocketPtr& pClient)
	{

	}


protected:
	CELLEpoll _ep;

};
#endif
