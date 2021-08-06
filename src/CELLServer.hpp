#ifndef _CELLServer_hpp_
#define _CELLServer_hpp_


#include<unistd.h> //uni std
#include<arpa/inet.h>
#include<string.h>
#include <memory>
#include<stdio.h>
#include<vector>
#include<thread>
#include<mutex>
#include<atomic>
#include <functional>

#include"MessageHeader.hpp"
#include"CELLTimestamp.hpp"
#include"CELLTask.hpp"
#include "CELLSemaphore.hpp"
#include "CELLLog.hpp"
#include "CELL.hpp"
#include "CELLNetWork.hpp"
#include "CELLClient.hpp"
#include "CELLMsgStream.hpp"
#include "CELLConfig.hpp"
// #include "CELLFDSet.hpp"
#include "CELLEpollServer.hpp"

typedef std::shared_ptr<ClientSocket> ClientSocketPtr;
class CellServer;

//网络事件接口
class INetEvent
{
public:
	//纯虚函数
	//客户端加入事件
	virtual void OnNetJoin(ClientSocketPtr& pClient) = 0;
	//客户端离开事件
	virtual void OnNetLeave(ClientSocketPtr& pClient) = 0;
	//客户端消息事件
	virtual void OnNetMsg(CellServer* pCellServer, ClientSocketPtr& pClient, DataHeader* header) = 0;
	//Recv事件
	virtual void OnNetRecv(ClientSocketPtr& pClient) = 0;
private:

};
// class CellSendMsg2ClientTask : public CellTask
// {
// private:
// 	ClientSocketPtr _pClient;
// 	std::shared_ptr<DataHeader> _pHeader;
// public:
// 	CellSendMsg2ClientTask(ClientSocketPtr& pClient, std::shared_ptr<DataHeader> pHeader)
// 	{
// 		_pClient = pClient;
// 		_pHeader = pHeader;
// 	}
// 	void doTask()
// 	{
// 		_pClient->SendData(_pHeader);
// 		// delete _pHeader;
// 	}

// };
class CellServer
{
public:

	virtual ~CellServer()
	{
		Close();
	}
	void setId(int id)
	{
		_id = id;
		_taskServer.serverId = id;
	}

	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	//关闭Socket
	void Close()
	{
		// _taskServer.Close();
		if (_thread.isRun())
		{
			_thread.Close();
			CELLLOG_Info("CellServer%d.close.", _id);
		}
	}

	//处理网络消息
	void OnRun(CELLThread* pThread)
	{
		_clients_change = true;	
		while (pThread->isRun())
		{
			if (_clientsBuff.size() > 0)
			{//从缓冲队列里取出客户数据
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients.push_back(pClient);
					pClient->serverId = _id;
					if (_pNetEvent)
					{
						_pNetEvent->OnNetJoin(pClient);
					}
					OnClientJoin(pClient);
				}
				_clientsBuff.clear();
				_clients_change = true;

			}

			//如果没有需要处理的客户端，就跳过
			if (_clients.empty())
			{
				_oldTime = CELLTime::getTimeInMilliSec();
				CELLThread::Sleep(1);
				continue;
			}

			// checkTime();
			if (!this->DoNetEvents())
			{
				pThread->Exit();
				break;
			}
			DoMsg();
		}
	}
	virtual bool DoNetEvents() = 0;
	/*{
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
	}*/

	// void clientWrite()
	// {
	// 	for (int n = (int)_clients.size() - 1; n >= 0; n--)
	// 	{
	// 		if (_fdWrite.has(_clients[n]->sockfd()))
	// 		{
	// 			if (-1 == _clients[n]->SendDataReal())
	// 			{
	// 				auto iter = _clients.begin() + n;//std::vector<SOCKET>::iterator
	// 				if (iter != _clients.end())
	// 				{
	// 					_clients_change = true;
	// 					if(_pNetEvent)
	// 						_pNetEvent->OnNetLeave(_clients[n]);
	// 					// delete _clients[n];
	// 					_clients.erase(iter);
	// 				}
	// 			}
	// 		}
	// 	}
	// }
	// void clientRead()
	// {
	// 	for (int n = (int)_clients.size() - 1; n >= 0; n--)
	// 	{
	// 		if (_fdRead.has(_clients[n]->sockfd()))
	// 		{
	// 			if (-1 == RecvData(_clients[n]))
	// 			{
	// 				auto iter = _clients.begin() + n;//std::vector<SOCKET>::iterator
	// 				if (iter != _clients.end())
	// 				{
	// 					_clients_change = true;
	// 					if(_pNetEvent)
	// 						_pNetEvent->OnNetLeave(_clients[n]);
	// 					// delete _clients[n];
	// 					_clients.erase(iter);
	// 				}
	// 			}
	// 		}
	// 	}
	// }
	
	void checkTime()
	{
		time_t nowTime = CELLTime::getTimeInMilliSec();
		time_t dt = nowTime - _oldTime;
		_oldTime = nowTime;
		if (dt > 60000)
		{
			CELLLOG_Info("dt=%d", dt);
		}
		
		for (int n = (int)_clients.size() - 1; n >= 0; n--)
		{
			// _clients[n]->checkSend(dt);
			if (_clients[n]->checkHeart(dt))
			{
				auto iter = _clients.begin() + n;//std::vector<SOCKET>::iterator
				if (iter != _clients.end())
				{
					_clients_change = true;
					if(_pNetEvent)
						_pNetEvent->OnNetLeave(_clients[n]);
					// delete _clients[n];
					_clients.erase(iter);
					CELLLOG_Info("sock=%d is close.", _clients[n]->sockfd());
				}
			}
		}
	}
	void DoMsg()
	{
		for (auto pClient : _clients)
		{
			while (pClient->hasMsg())
			{
				OnNetMsg(pClient, pClient->frontMsg());
				pClient->popFrontMsg();
			}
		}
	}
	//接收数据 处理粘包 拆分包
	int RecvData(ClientSocketPtr& pClient)
	{
		// 5 接收客户端数据
		int nLen = pClient->RecvData();
		// static std::atomic_int y(0);
		// printf("RecvData = %d\n", (int)y++);
		_pNetEvent->OnNetRecv(pClient);
		return nLen;
	}

	//响应网络消息
	void OnNetMsg(ClientSocketPtr& pClient, DataHeader* header)
	{
		_pNetEvent->OnNetMsg(this, pClient, header);
	}
	virtual void OnClientJoin(ClientSocketPtr& pClient)
	{

	}


	void addClient(ClientSocketPtr& pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}

	void Start()
	{
		_thread.Start(nullptr, [this](CELLThread* pThread){
            this->OnRun(pThread);}, [this](CELLThread* pThread){
				_clients.clear();
				_clientsBuff.clear();	
			});
		// _taskServer.Start();
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}
	void addSendTask(ClientSocketPtr& pClient, std::shared_ptr<DataHeader> header /*DataHeader* header*/)
	{
		// auto task = std::make_shared<CellSendMsg2ClientTask>(pClient, header);
		// CellSendMsg2ClientTask* task = new CellSendMsg2ClientTask(pClient, header);
		_taskServer.addTask([pClient, header](){pClient->SendData(header);});
	}
private:

	//缓冲客户队列
	std::vector<ClientSocketPtr> _clientsBuff;
	//缓冲队列的锁
	std::mutex _mutex;
	CellTaskServer _taskServer;
	time_t _oldTime = CELLTime::getTimeInMilliSec();
	
	CELLThread _thread;
	// CELLFDSet _fdRead;//描述符（socket） 集合
	// CELLFDSet _fdWrite;
	// CELLFDSet _fdRead_bak;
	SOCKET _maxSock;
protected:
	//正式客户队列
	std::vector<ClientSocketPtr> _clients;
	bool _clients_change;
	int _id = -1;
	//网络事件对象
	INetEvent* _pNetEvent = nullptr;
};

#endif // !_CELLServer_hpp_
