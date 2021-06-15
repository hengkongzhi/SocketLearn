#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_


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
#include "CELLClient.hpp"
#include "CELLMsgStream.hpp"



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
	CellServer(int id)
	{
		_id = id;
		_pNetEvent = nullptr;
		_taskServer.serverId = id;
	}

	~CellServer()
	{
		Close();
	}

	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	//关闭Socket
	void Close()
	{
		// _taskServer.Close();
		_thread.Close();
		CELLLOG_Info("CellServer%d.close.", _id);

	}

	//处理网络消息
	bool OnRun(CELLThread* pThread)
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
				}
				_clientsBuff.clear();
				_clients_change = true;

			}

			//如果没有需要处理的客户端，就跳过
			if (_clients.empty())
			{
				_oldTime = CELLTime::getTimeInMilliSec();
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}

			checkTime();
			//伯克利套接字 BSD socket
			fd_set fdRead;//描述符（socket） 集合
			fd_set fdWrite;
			// fd_set fdExp;
			//清理集合
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			// FD_ZERO(&fdExp);
			//将描述符（socket）加入集合
			
			if (_clients_change)
			{
				_clients_change = false;
				_maxSock = _clients[0]->sockfd();
				for (int n = (int)_clients.size() - 1; n >= 0; n--)
				{
					FD_SET(_clients[n]->sockfd(), &fdRead);
					if (_maxSock < _clients[n]->sockfd())
					{
						_maxSock = _clients[n]->sockfd();
					}
				}
				memcpy(&_fdRead_bak, &fdRead, sizeof(fd_set));
			}
			else
			{
				memcpy(&fdRead, &_fdRead_bak, sizeof(fd_set));
			}

			bool bNeedWrite = false;
			for (auto pClient : _clients)
			{
				if (pClient->NeedWrite())
				{
					bNeedWrite = true;
					FD_SET(pClient->sockfd(), &fdWrite);
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
				ret = select(_maxSock + 1, &fdRead, &fdWrite, nullptr, &t);
			}
			else
			{
				ret = select(_maxSock + 1, &fdRead, nullptr, nullptr, &t);
			}
			if (ret < 0)
			{
				CELLLOG_Info("Cellserver.OnRun select error.");
				pThread->Exit();
				return false;
			}
			else if (ret == 0)
			{
				continue;
			}

			clientRead(fdRead);
			clientWrite(fdWrite);
		}
	}
	void clientWrite(fd_set& fdWrite)
	{
		for (int n = (int)_clients.size() - 1; n >= 0; n--)
		{
			if (FD_ISSET(_clients[n]->sockfd(), &fdWrite))
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
	void clientRead(fd_set& fdRead)
	{
		for (int n = (int)_clients.size() - 1; n >= 0; n--)
		{
			if (FD_ISSET(_clients[n]->sockfd(), &fdRead))
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

	//接收数据 处理粘包 拆分包
	int RecvData(ClientSocketPtr& pClient)
	{
		// 5 接收客户端数据
		int nLen = pClient->RecvData();
		_pNetEvent->OnNetRecv(pClient);
		//CELLLOG_Info("nLen=%d", nLen);
		if (nLen <= 0)
		{
			CELLLOG_Info("客户端<Socket=%d>已退出，任务结束。", pClient->sockfd());
			return -1;
		}
		while (pClient->hasMsg())
		{
			OnNetMsg(pClient, pClient->frontMsg());
			pClient->popFrontMsg();
		}
		return 0;
	}

	//响应网络消息
	void OnNetMsg(ClientSocketPtr& pClient, DataHeader* header)
	{
		_pNetEvent->OnNetMsg(this, pClient, header);
		switch (header->cmd)
		{
			case CMD_LOGIN:
			{
				pClient->resetDTHeart();
				Login* login = (Login*)header;
				//CELLLOG_Info("收到客户端<Socket=%d>请求：CMD_LOGIN,数据长度：%d,userName=%s PassWord=%s", cSock, login->dataLength, login->userName, login->PassWord);
				//忽略判断用户密码是否正确的过程
				// LoginResult *ret = new LoginResult();
				std::shared_ptr<LoginResult> ret = std::make_shared<LoginResult>();
				// SOCKET xre = 100;
				// xre = 
				if (pClient->SendData(ret) == SOCKET_ERROR)
				{
					CELLLOG_Info("Sendbuffer is full");
				}
				// return xre;
				// this->addSendTask(pClient, ret);
			}
			break;
			case CMD_LOGOUT:
			{
				CELLRecvMsgStream r(header);
                auto n1 = r.ReadInt8();
                auto n2 = r.ReadInt16();
                auto n3 = r.ReadInt32();
                auto n4 = r.ReadFloat();
                auto n5 = r.ReadDouble();
                uint32_t n = 0;
                r.onlyRead(n);
                char name[32] = {0};
                r.ReadArray(name, 32);
                char pwd[32] = {0};
                r.ReadArray(pwd, 32);
                int data[10] = {0};
                r.ReadArray(data, 10);

				CELLSendMsgStream s;
    			s.setNetCmd(CMD_LOGOUT_RESULT);
    			s.WriteInt8(5);
    			s.WriteInt16(5);
    			s.WriteInt32(5.0f);
    			s.WriteFloat(5.0f);
    			s.WriteDouble(5.0f);
    			const char* str = "helloworld";
    			s.WriteArray(str, strlen(str));
    			char a[] = "asdsa";
    			s.WriteArray(a, strlen(a));
    			int b[] = {1, 2, 3, 4, 5};
    			s.WriteArray(b, 5);
    			s.finsh();
				pClient->SendData(s.data(), s.length());
				//CELLLOG_Info("收到客户端<Socket=%d>请求：CMD_LOGOUT,数据长度：%d,userName=%s ", cSock, logout->dataLength, logout->userName);
				//忽略判断用户密码是否正确的过程
				//LogoutResult ret;
				//SendData(cSock, &ret);
			}
			break;
			case CMD_C2S_HEART:
			{
				pClient->resetDTHeart();
				std::shared_ptr<s2c_Heart> ret = std::make_shared<s2c_Heart>();
				pClient->SendData(ret);

			}
			default:
			{
				CELLLOG_Info("<socket=%d>收到未定义消息,数据长度：%d", pClient->sockfd(), header->dataLength);
				//DataHeader ret;
				//SendData(cSock, &ret);
			}
			break;
		}
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
            OnRun(pThread);}, [this](CELLThread* pThread){
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
	//正式客户队列
	std::vector<ClientSocketPtr> _clients;
	//缓冲客户队列
	std::vector<ClientSocketPtr> _clientsBuff;
	//缓冲队列的锁
	std::mutex _mutex;

	//网络事件对象
	INetEvent* _pNetEvent;

	CellTaskServer _taskServer;

	fd_set _fdRead_bak;
	bool _clients_change;
	SOCKET _maxSock;
	time_t _oldTime = CELLTime::getTimeInMilliSec();
	int _id = -1;
	CELLThread _thread;
};

class EasyTcpServer : public INetEvent
{
private:
	SOCKET _sock;
	//消息处理对象，内部会创建线程
	std::vector<std::shared_ptr<CellServer>> _cellServers;
	//每秒消息计时
	CELLTimestamp _tTime;
	CELLThread _thread;
protected:
	//收到消息计数
	std::atomic_int _recvCount;
	//客户端计数
	std::atomic_int _clientCount;
	std::atomic_int _msgCount;
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
		_recvCount = 0;
		_clientCount = 0;
		_msgCount = 0;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}
	//初始化Socket
	SOCKET InitSocket()
	{
		if (INVALID_SOCKET != _sock)
		{
			CELLLOG_Info("<socket=%d>关闭旧连接...", (int)_sock);
			close(_sock);
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			CELLLOG_Info("错误，建立socket失败...");
		}
		else {
			//printf("建立socket=<%d>成功...\n", (int)_sock);
			CELLLOG_Info("建立socket=<%d>成功...", (int)_sock);
		}
		return _sock;
	}

	//绑定IP和端口号
	int Bind(const char* ip, unsigned short port)
	{
		//if (INVALID_SOCKET == _sock)
		//{
		//	InitSocket();
		//}
		// 2 bind 绑定用于接受客户端连接的网络端口
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);//host to net unsigned short

		if (ip) {
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret)
		{
			CELLLOG_Info("错误,绑定网络端口<%d>失败...", port);
		}
		else {
			CELLLOG_Info("绑定网络端口<%d>成功...", port);
		}
		return ret;
	}

	//监听端口号
	int Listen(int n)
	{
		// 3 listen 监听网络端口
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)
		{
			CELLLOG_Info("socket=<%d>错误,监听网络端口失败...",_sock);
		}
		else {
			CELLLOG_Info("socket=<%d>监听网络端口成功...", _sock);
		}
		return ret;
	}

	//接受客户端连接
	SOCKET Accept()
	{
		// 4 accept 等待接受客户端连接
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t *)&nAddrLen);
		if (INVALID_SOCKET == cSock)
		{
			//printf("socket=<%d>错误,接受到无效客户端SOCKET...\n", (int)_sock);
			CELLLOG_Info("socket=<%d>错误,接受到无效客户端SOCKET...", (int)_sock);
		}
		else
		{
			//printf("接受到客户端cSocket=<%d>...\n", (int)cSock);
			//将新客户端分配给客户数量最少的cellServer
			std::shared_ptr<ClientSocket> tmp(new ClientSocket(cSock));
			addClientToCellServer(tmp /*new ClientSocket(cSock)*/);
			//获取IP地址 inet_ntoa(clientAddr.sin_addr)
		}
		return cSock;
	}
	
	void addClientToCellServer(ClientSocketPtr& pClient)
	{
		//查找客户数量最少的CellServer消息处理对象
		auto pMinServer = _cellServers[0];
		for(auto pCellServer : _cellServers)
		{
			if (pMinServer->getClientCount() > pCellServer->getClientCount())
			{
				pMinServer = pCellServer;
			}
		}
		pMinServer->addClient(pClient);
		OnNetJoin(pClient);
	}

	void Start(int nCellServer)
	{
		for (int n = 0; n < nCellServer; n++)
		{
			auto ser = std::make_shared<CellServer>(n + 1);
			// auto ser = new CellServer(_sock);
			_cellServers.push_back(ser);
			//注册网络事件接受对象
			ser->setEventObj(this);
			//启动消息处理线程
			ser->Start();
		}
		_thread.Start(nullptr, [this](CELLThread* pThread){
        OnRun(pThread);});
	}
	//关闭Socket
	void Close()
	{
		_thread.Close();
		if (_sock != INVALID_SOCKET)
		{
			//关闭套节字closesocket
			close(_sock);
		}
	}


	//计算并输出每秒收到的网络消息
	void time4msg()
	{
		auto t1 = _tTime.getElapsedSecond();
		if (t1 >= 1.0)
		{
			//printf("thread<%d>,time<%lf>,socket<%d>,clients<%d>,msgCount<%d>,recvCount<%d>\n", _cellServers.size(), t1, _sock,(int)_clientCount, (int)(_msgCount/ t1), (int)_recvCount);
			CELLLOG_Info("thread<%d>,time<%lf>,socket<%d>,clients<%d>,msgCount<%d>,recvCount<%d>", _cellServers.size(), t1, _sock,(int)_clientCount, (int)(_msgCount/ t1), (int)_recvCount);
			_recvCount = 0;
			_msgCount = 0;
			_tTime.update();
		}
	}
	//只会被一个线程触发 安全
	virtual void OnNetJoin(ClientSocketPtr& pClient)
	{
		_clientCount++;
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetLeave(ClientSocketPtr& pClient)
	{
		_clientCount--;
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetMsg(CellServer* pCellServer, ClientSocketPtr& pClient, DataHeader* header)
	{
		_msgCount++;
	}
	virtual void OnNetRecv(ClientSocketPtr& pClient)
	{
		_recvCount++;
	}
private:
	//处理网络消息
	void OnRun(CELLThread* pThread)
	{
		while (pThread->isRun())
		{
			time4msg();
			//伯克利套接字 BSD socket
			fd_set fdRead;//描述符（socket） 集合
			//清理集合
			FD_ZERO(&fdRead);
			//将描述符（socket）加入集合
			FD_SET(_sock, &fdRead);
			///nfds 是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
			///既是所有文件描述符最大值+1 在Windows中这个参数可以写0
			timeval t = { 0,1};
			int ret = select(_sock + 1, &fdRead, 0, 0, &t); //
			if (ret < 0)
			{
				CELLLOG_Info("EasyTcpServer.onRun Select任务结束。");
				pThread->Exit();
				break;
			}
			//判断描述符（socket）是否在集合中
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
			}
		}
	}
};

#endif // !_EasyTcpServer_hpp_
