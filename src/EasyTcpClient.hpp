#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

#include<unistd.h> //uni std
#include<arpa/inet.h>
#include<string.h>
#include <stdio.h>
#include "MessageHeader.hpp"
#include "CELLLog.hpp"
#include "CELL.hpp"
#include "CELLClient.hpp"

class EasyTcpClient
{

public:
	EasyTcpClient()
	{
		_isConnect = false;
		_pClient = nullptr;
	}
	
	virtual ~EasyTcpClient()
	{
		Close();
	}
	//初始化socket
	SOCKET InitSocket()
	{
		if (_pClient)
		{
			CELLLOG_Info("<socket=%d>关闭旧连接...\n", _pClient->sockfd());
			Close();
		}
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == sock)
		{
			CELLLOG_Info("错误，建立Socket失败...\n");
		}
		else {
			_pClient = new ClientSocket(sock);
			//CELLLOG_Info("建立Socket=<%d>成功...\n", _sock);
		}
		return sock;
	}

	//连接服务器
	int Connect(const char* ip,unsigned short port)
	{
		if (!_pClient)
		{
			InitSocket();
		}
		// 2 连接服务器 connect
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);

		_sin.sin_addr.s_addr = inet_addr(ip);

		//CELLLOG_Info("<socket=%d>正在连接服务器<%s:%d>...\n", _sock, ip, port);
		int ret = connect(_pClient->sockfd(), (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			CELLLOG_Info("<socket=%d>错误，连接服务器<%s:%d>失败...\n", _pClient->sockfd(), ip, port);
		}
		else {
			_isConnect = true;
			//CELLLOG_Info("<socket=%d>连接服务器<%s:%d>成功...\n",_sock, ip, port);
		}
		return ret;
	}

	//关闭套节字closesocket
	void Close()
	{
		if (_pClient)
		{
			close(_pClient->sockfd());
			delete _pClient;
			_pClient = nullptr;
		}
		_isConnect = false;
	}

	//处理网络消息
	bool OnRun()
	{

		if (isRun())
		{
			SOCKET _sock = _pClient->sockfd();

			fd_set fdRead;
			fd_set fdWrite;
			fd_set* pfdW;
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_SET(_sock, &fdRead);
			if (_pClient->NeedWrite())
			{
				FD_SET(_sock, &fdWrite);
				pfdW = &fdWrite;
			}
			else
			{
				pfdW = nullptr;
			}
			timeval t = { 0,0 };
			int ret = select(_sock + 1, &fdRead, pfdW, 0, &t); 
			if (ret < 0)
			{
				CELLLOG_Info("<socket=%d>select任务结束1\n", _sock);
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdRead))
			{

				if (-1 == RecvData(_sock))
				{
					CELLLOG_Info("<socket=%d>select任务结束2\n", _sock);
					Close();
					return false;
				}
			}
			if (pfdW != nullptr)
			{
				if (FD_ISSET(_sock, &fdWrite))
				{
					if (-1 == _pClient->SendDataReal())
					{
						CELLLOG_Info("<socket=%d>select任务结束2\n", _sock);
						Close();
						return false;
					}
				}
			}
			return true;
		}
		return false;
	}

	//是否工作中
	bool isRun()
	{
		return _pClient && _isConnect;
	}
	//缓冲区最小单元大小
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 10240 * 3
#endif // !RECV_BUFF_SZIE
	//第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SZIE] = {};
	//消息缓冲区的数据尾部位置
	int _lastPos = 0;

	//接收数据 处理粘包 拆分包
	int RecvData(SOCKET cSock)
	{
		// // 5 接收数据
		// char* szRecv = _szMsgBuf + _lastPos;
		// int nLen = (int)recv(cSock, szRecv, RECV_BUFF_SZIE - _lastPos, 0);
		// //CELLLOG_Info("nLen=%d\n", nLen);
		// if (nLen < 0)
		// {
		// 	CELLLOG_Info("<socket=%d>与服务器断开连接，任务结束。\n", cSock);
		// 	return -1;
		// }
		// //将收取到的数据拷贝到消息缓冲区
		// //memcpy(_szMsgBuf+_lastPos, _szRecv, nLen);
		// //消息缓冲区的数据尾部位置后移
		// _lastPos += nLen;
		// //判断消息缓冲区的数据长度大于消息头DataHeader长度
		// while (_lastPos >= sizeof(DataHeader))
		// {
		// 	//这时就可以知道当前消息的长度
		// 	DataHeader* header = (DataHeader*)_szMsgBuf;
		// 	//判断消息缓冲区的数据长度大于消息长度
		// 	if (_lastPos >= header->dataLength)
		// 	{
		// 		//消息缓冲区剩余未处理数据的长度
		// 		int nSize = _lastPos - header->dataLength;
		// 		//处理网络消息
		// 		OnNetMsg(header);
		// 		//将消息缓冲区剩余未处理数据前移
		// 		memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, nSize);
		// 		//消息缓冲区的数据尾部位置前移
		// 		_lastPos = nSize;
		// 	}
		// 	else {
		// 		//消息缓冲区剩余数据不够一条完整消息
		// 		break;
		// 	}
		// }
		// return 0;

		if (isRun())
		{
			int nLen = _pClient->RecvData();
			//CELLLOG_Info("nLen=%d\n", nLen);
			if (nLen > 0)
			{
				while (_pClient->hasMsg())
				{
					OnNetMsg(_pClient->frontMsg());
					_pClient->popFrontMsg();
				}
			}
			return nLen;
		}
		return 0;
	}

	//响应网络消息
	virtual void OnNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
			case CMD_LOGIN_RESULT:
			{
			
				LoginResult* login = (LoginResult*)header;
				//CELLLOG_Info("<socket=%d>收到服务端消息：CMD_LOGIN_RESULT,数据长度：%d\n", _sock, login->dataLength);
			}
			break;
			case CMD_LOGOUT_RESULT:
			{
				LogoutResult* logout = (LogoutResult*)header;
				//CELLLOG_Info("<socket=%d>收到服务端消息：CMD_LOGOUT_RESULT,数据长度：%d\n", _sock, logout->dataLength);
			}
			break;
			case CMD_NEW_USER_JOIN:
			{
				NewUserJoin* userJoin = (NewUserJoin*)header;
				//CELLLOG_Info("<socket=%d>收到服务端消息：CMD_NEW_USER_JOIN,数据长度：%d\n", _sock, userJoin->dataLength);
			}
			break;
			case CMD_ERROR:
			{
				CELLLOG_Info("<socket=%d>收到服务端消息：CMD_ERROR,数据长度：%d\n", _pClient->sockfd(), header->dataLength);
			}
			break;
			default:
			{
				CELLLOG_Info("<socket=%d>收到未定义消息,数据长度：%d\n", _pClient->sockfd(), header->dataLength);
			}
		}
	}

	//发送数据
	int SendData(std::shared_ptr<DataHeader> header)
	{
		// int ret = SOCKET_ERROR;
		// if (isRun() && header)
		// {
		// 	ret = send(_sock, (const char*)header, nLen, 0);
		// 	if (SOCKET_ERROR == ret)
		// 	{
		// 		Close();
		// 	}
		// }
		if (isRun())
		{
			return _pClient->SendData(header);
		}
		return 0;
	}
	int SendData(const char* pData, int len)
	{
		if (isRun())
		{
			return _pClient->SendData(pData, len);
		}
		return 0;
	}
private:
	ClientSocket* _pClient;
	bool _isConnect;
};

#endif