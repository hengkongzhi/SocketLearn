#ifndef _EasyTcpSelectClient_hpp_
#define _EasyTcpSelectClient_hpp_

#include "EasyTcpClient.hpp"
#include "CELLFDSet.hpp"

class EasyTcpSelectClient : public EasyTcpClient
{

public:
	EasyTcpSelectClient()
	{
		_fdRead.create(1);
		_fdWrite.create(1);
	}
	//处理网络消息
	bool OnRun(int microseconds = 1)
	{

		if (isRun())
		{
			SOCKET _sock = _pClient->sockfd();
			_fdRead.zero();
			_fdWrite.zero();
			_fdRead.add(_sock);
			timeval t = {0, microseconds};
			int ret = 0;
			if (_pClient->NeedWrite())
			{
				_fdWrite.add(_sock);
				ret = select(_sock + 1, _fdRead.fdset(), _fdWrite.fdset(), 0, &t); 
			}
			else
			{
				ret = select(_sock + 1, _fdRead.fdset(), nullptr, 0, &t); 
			}
			if (ret < 0)
			{
				CELLLOG_PError("<socket=%d>select exit...", _sock);
				Close();
				return false;
			}
			if (_fdRead.has(_sock))
			{

				if (-1 == RecvData(_sock))
				{
					CELLLOG_PError("<socket=%d>select RecvData exit...", _sock);
					Close();
					return false;
				}
			}
			if (_fdWrite.has(_sock))
			{
				if (-1 == _pClient->SendDataReal())
				{
					CELLLOG_PError("<socket=%d>select SendDataReal exit...", _sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}
protected:
	CELLFDSet _fdRead;
	CELLFDSet _fdWrite;
};

#endif