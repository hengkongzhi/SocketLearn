#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#include<string.h>
#include <memory>
#include<stdio.h>
#include<vector>
#include<thread>
#include<mutex>
#include<atomic>
#include <functional>
#include "CELLNetWork.hpp"
#include"MessageHeader.hpp"
#include"CELLTimestamp.hpp"
#include"CELLTask.hpp"
#include "CELLSemaphore.hpp"
#include "CELLLog.hpp"
#include "CELL.hpp"
#include "CELLClient.hpp"
#include "CELLMsgStream.hpp"
#include "CELLConfig.hpp"
#include "EasyTcpEpollServer.hpp"
#include "CELLEpollServer.hpp"

class EasyTcpServer : public INetEvent
{
private:
	SOCKET _sock;
	//消息处理对象，内部会创建线程
	// std::vector<std::shared_ptr<CellServer>> _cellServers;
	std::vector<CellServer*> _cellServers;
	//每秒消息计时
	CELLTimestamp _tTime;
	CELLThread _thread;
	int _nSendBuffSize;
	int _nRecvBuffSize;
	int _nMaxClient;
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
		_nSendBuffSize = CELLConfig::Instance().getInt("nSendBuffSize", SEND_BUFF_SZIE);
		_nRecvBuffSize = CELLConfig::Instance().getInt("nRecvBuffSize", RECV_BUFF_SZIE);
		_nMaxClient = CELLConfig::Instance().getInt("nMaxClient", 10240);
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
			CELLLOG_Warring("InitSocket close old socket<%d>...", (int)_sock);
			CELLNetWork::destroy(_sock);
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			CELLLOG_PError("create socket failed...");
		}
		else {
			int flag = 1;
			if (SOCKET_ERROR == setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(flag)))
			{
				CELLLOG_PError("setsockopt SO_REUSEADDR failed...");
			}
			//printf("建立socket=<%d>成功...\n", (int)_sock);
			CELLLOG_Info("create socket=<%d> success...", (int)_sock);
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
			CELLLOG_PError("bind port<%d> failed...", port);
		}
		else {
			CELLLOG_Info("bind port<%d> success...", port);
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
			CELLLOG_PError("listen socket=<%d> failed...", _sock);
		}
		else {
			CELLLOG_Info("listen socket=<%d> success...", _sock);
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
			CELLLOG_PError("accept INVALID_SOCKET...");
		}
		else
		{
			//printf("接受到客户端cSocket=<%d>...\n", (int)cSock);
			//将新客户端分配给客户数量最少的cellServer
			if (_clientCount < _nMaxClient)
			{
				std::shared_ptr<ClientSocket> tmp(new ClientSocket(cSock, _nSendBuffSize, _nRecvBuffSize));
				addClientToCellServer(tmp /*new ClientSocket(cSock)*/);
			}
			else
			{
				CELLNetWork::destroy(cSock);
				CELLLOG_Warring("Accept to nMaxClient");
			}

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
		//OnNetJoin(pClient);
	}
	template<typename ServerT>
	void Start(int nCellServer)
	{
		for (int n = 0; n < nCellServer; n++)
		{
			// auto ser = std::make_shared<ServerT>();
			ServerT* ser = new ServerT();
			ser->setId(n + 1);
			// auto ser = new CellServer(_sock);
			_cellServers.push_back(ser);
			//注册网络事件接受对象
			ser->setEventObj(this);
			//启动消息处理线程
			ser->Start();
		}
		_thread.Start(nullptr, [this](CELLThread* pThread){
        this->OnRun(pThread);});
	}
	//关闭Socket
	void Close()
	{
		for (auto it : _cellServers)
		{
			delete it;
		}
		_cellServers.clear();
		_thread.Close();
		if (_sock != INVALID_SOCKET)
		{
			//关闭套节字closesocket
			CELLNetWork::destroy(_sock);
		}
	}



	//计算并输出每秒收到的网络消息
	void time4msg()
	{
		auto t1 = _tTime.getElapsedSecond();
		if (t1 >= 1.0)
		{
			//printf("thread<%d>,time<%lf>,socket<%d>,clients<%d>,msgCount<%d>,recvCount<%d>\n", _cellServers.size(), t1, _sock,(int)_clientCount, (int)(_msgCount/ t1), (int)_recvCount);
			CELLLOG_Info("thread<%d>,time<%lf>,socket<%d>,clients<%d>,msgCount<%d>,recvCount<%d>", _cellServers.size(), t1, _sock,(int)_clientCount, (int)(_msgCount), (int)_recvCount);
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
protected:
	//处理网络消息
	virtual void OnRun(CELLThread* pThread)
	{
		// CELLFDSet fdRead;
		// // fd_set fdRead;//描述符（socket） 集合
		// while (pThread->isRun())
		// {
		// 	time4msg();
		// 	//伯克利套接字 BSD socket
			
		// 	//清理集合
		// 	fdRead.zero();
		// 	// FD_ZERO(&fdRead);
		// 	//将描述符（socket）加入集合
		// 	fdRead.add(_sock);
		// 	// FD_SET(_sock, &fdRead);
		// 	///nfds 是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
		// 	///既是所有文件描述符最大值+1 在Windows中这个参数可以写0
		// 	timeval t = { 0,1};
		// 	int ret = select(_sock + 1, fdRead.fdset(), 0, 0, &t); //
		// 	if (ret < 0)
		// 	{
		// 		CELLLOG_Info("EasyTcpServer.onRun Select任务结束。");
		// 		pThread->Exit();
		// 		break;
		// 	}
		// 	//判断描述符（socket）是否在集合中
		// 	if (fdRead.has(_sock))
		// 	{
		// 		fdRead.del(_sock);
		// 		// FD_CLR(_sock, &fdRead);
		// 		Accept();
		// 	}
		// }
	}
	int sockfd()
	{
		return _sock;
	}
};

#endif // !_EasyTcpServer_hpp_
