#ifndef _EasyTcpEpollClient_hpp_
#define _EasyTcpEpollClient_hpp_

#include "EasyTcpClient.hpp"
#include "CELLEpoll.hpp"
class EasyTcpEpollClient : public EasyTcpClient
{
public:
	virtual void OnInitSocket()
	{
		_ep.create(1);
		_ep.cellEpollCtl(EPOLL_CTL_ADD, _pClient, EPOLLIN);
	}
	void Close()
	{
		_ep.destroy();
		EasyTcpClient::Close();
	}
	//处理网络消息
	bool OnRun(int microseconds = 1)
	{

		if (isRun())
		{
			if (_pClient->NeedWrite())
			{
				_ep.cellEpollCtl(EPOLL_CTL_MOD, _pClient, EPOLLOUT | EPOLLIN);
			}
			else
			{
				_ep.cellEpollCtl(EPOLL_CTL_MOD, _pClient, EPOLLIN);
			}
			int ret = _ep.wait(microseconds);
			if (ret < 0)
			{
				CELLLOG_Error("EasyTcpEpollClient.wait error.");
				return false;
			}
			else if (ret == 0)
			{
				return true;
			}
			auto events = _ep.events();
			ClientSocket* pclient = (ClientSocket*)events->data.ptr;
        	if (pclient)
        	{
				if (events->events & EPOLLIN)
				{
					if (-1 == RecvData(pclient->sockfd()))
					{
						CELLLOG_PError("<socket=%d>epoll RecvData exit...", pclient->sockfd());
						Close();
						return false;
					}
				}
				if (events->events & EPOLLOUT)
				{
					if (-1 == pclient->SendDataReal())
					{
						CELLLOG_PError("<socket=%d>epoll SendDataReal exit...", pclient->sockfd());
						Close();
						return false;
					}
				}
			}
			return true;
		}
		return false;
	}
protected:
	CELLEpoll _ep;
};

#endif