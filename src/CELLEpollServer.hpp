#ifndef _CELLEpollServer_hpp_
#define _CELLEpollServer_hpp_

#include "CELLServer.hpp"
#include "CELLEpoll.hpp"
#include <vector>
#include <algorithm>

class CellEpollServer : public CellServer
{
public:
	~CellEpollServer()
	{
		Close();
	}
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
		bool bNeedWrite = false;
		for (auto pClient : _clients)
		{
			if (pClient->NeedWrite())
			{
				_ep.cellEpollCtl(EPOLL_CTL_MOD, pClient, EPOLLOUT | EPOLLIN);
				// FD_SET(pClient->sockfd(), &fdWrite);
			}
			else
			{
				_ep.cellEpollCtl(EPOLL_CTL_MOD, pClient, EPOLLIN);
			}
		}
		int ret = _ep.wait(0);
		if (ret < 0)
		{
			CELLLOG_Error("CellEpollServer.wait error.");
			return false;
		}
		else if (ret == 0)
		{
			return true;
		}
		else
		{
			printf("ret is %d, id is %d\n", ret, _id);
		}
		auto events = _ep.events();
		for (int i = 0; i < ret; i++)
        {
			ClientSocketPtr& pclient = *(ClientSocketPtr*)events[i].data.ptr;
        	if (pclient.get())
        	{
				if (events[i].events & EPOLLIN)
				{
					if (-1 == RecvData(pclient))
					{
						rmClient(pclient);
						continue;
					}
					// static std::atomic_int x(0);
					// printf("EPOLLIN = %d\n", (int)x++);
				}
				if (events[i].events & EPOLLOUT)
				{
					if (-1 ==  pclient->SendDataReal())
					{
						rmClient(pclient);
					}
					// static std::atomic_int y(0);
					// printf("EPOLLOUT = %d\n", (int)y++);
				}
			}
		}
		return true;
	}
	void rmClient(ClientSocketPtr& pClient)
	{
		if (_pNetEvent)
		{
			_pNetEvent->OnNetLeave(pClient);
		}
		auto iter = find(_clients.begin(), _clients.end(), pClient);
		if (iter != _clients.end())
		{
			_clients.erase(iter);
		}
		
	}


protected:
	CELLEpoll _ep;

};
#endif
