#ifndef _CELLSelectServer_hpp_
#define _CELLSelectServer_hpp_

#include "EasyTcpServer.hpp"

class CellSelectServer : public CellServer
{
public:
	virtual bool DoNetEvents()
	{
		//伯克利套接字 BSD socket

		// fd_set fdExp;
		//清理集合
		_fdRead.zero();
		_fdWrite.zero();
		// FD_ZERO(&fdRead);
		// FD_ZERO(&fdWrite);
		// FD_ZERO(&fdExp);
		//将描述符（socket）加入集合
		
		if (_clients_change)
		{
			_clients_change = false;
			_maxSock = _clients[0]->sockfd();
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				_fdRead.add(_clients[n]->sockfd());
				// FD_SET(_clients[n]->sockfd(), &fdRead);
				if (_maxSock < _clients[n]->sockfd())
				{
					_maxSock = _clients[n]->sockfd();
				}
			}
			_fdRead_bak.copy(_fdRead);
		}
		else
		{
			_fdRead.copy(_fdRead_bak);
		}
		bool bNeedWrite = false;
		for (auto pClient : _clients)
		{
			if (pClient->NeedWrite())
			{
				bNeedWrite = true;
				_fdWrite.add(pClient->sockfd());
				// FD_SET(pClient->sockfd(), &fdWrite);
			}
		}
		// memcpy(&fdWrite, &_fdRead_bak, sizeof(fd_set));
		// memcpy(&fdExp, &_fdRead_bak, sizeof(fd_set));
		
		///nfds 是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
		///既是所有文件描述符最大值+1 在Windows中这个参数可以写0
		timeval t{0, 1};
		int ret = 0;
		if (bNeedWrite)
		{
			ret = select(_maxSock + 1, _fdRead.fdset(), _fdWrite.fdset(), nullptr, &t);
		}
		else
		{
			ret = select(_maxSock + 1, _fdRead.fdset(), nullptr, nullptr, &t);
		}
		if (ret < 0)
		{
			CELLLOG_Info("Cellserver.OnRun select error.");
			return false;
		}
		else if (ret == 0)
		{
			return true;
		}
		clientRead();
		clientWrite();
		return true;
	}

	void clientWrite()
	{
		for (int n = (int)_clients.size() - 1; n >= 0; n--)
		{
			if (_fdWrite.has(_clients[n]->sockfd()))
			{
				if (-1 == _clients[n]->SendDataReal())
				{
					auto iter = _clients.begin() + n;//std::vector<SOCKET>::iterator
					if (iter != _clients.end())
					{
						_clients_change = true;
						if(_pNetEvent)
							_pNetEvent->OnNetLeave(_clients[n]);
						// delete _clients[n];
						_clients.erase(iter);
					}
				}
			}
		}
	}
	void clientRead()
	{
		for (int n = (int)_clients.size() - 1; n >= 0; n--)
		{
			if (_fdRead.has(_clients[n]->sockfd()))
			{
				if (-1 == RecvData(_clients[n]))
				{
					auto iter = _clients.begin() + n;//std::vector<SOCKET>::iterator
					if (iter != _clients.end())
					{
						_clients_change = true;
						if(_pNetEvent)
							_pNetEvent->OnNetLeave(_clients[n]);
						// delete _clients[n];
						_clients.erase(iter);
					}
				}
			}
		}
	}
protected:
	CELLFDSet _fdRead;//描述符（socket） 集合
	CELLFDSet _fdWrite;
	CELLFDSet _fdRead_bak;
	SOCKET _maxSock;

};
#endif
