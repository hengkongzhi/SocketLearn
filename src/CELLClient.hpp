#ifndef _CELL_CLIENT_HPP_
#define _CELL_CLIENT_HPP_
#include "CELLBuffer.hpp"
#include "CELL.hpp"
//缓冲区最小单元大小
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 1024 * 1
#define SEND_BUFF_SZIE 1024 * 1
#endif // !RECV_BUFF_SZIE

#define CLIENT_HEART_DEAD_TIME 60000
#define CLIENT_SEND_BUFF_TIME 200
//客户端数据类型
class ClientSocket /*: public ObjectPoolBase<ClientSocket, 10000>*/
{
public:
	int id = -1;
	int serverId = -1;
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET) : _sendBuff(SEND_BUFF_SZIE), _recvBuff(RECV_BUFF_SZIE)
	{
		static int n = 1;
		id = n++;
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, RECV_BUFF_SZIE);
		_lastPos = 0;
		resetDTHeart();
		resetDTSend();
		_isFull = false;
	}
	~ClientSocket()
	{
		CELLLog::Info("s=%d ClientSocket%d.~ClientSocket\n", serverId, id);
		close(_sockfd);
	}

	SOCKET sockfd()
	{
		return _sockfd;
	}
	int RecvData()
	{
		return _recvBuff.read4socket(_sockfd);
	}
	bool hasMsg()
	{
		return _recvBuff.hasMsg();
	}
	DataHeader* frontMsg()
	{
		return (DataHeader*)_recvBuff.data();
	}
	void popFrontMsg()
	{
		if (hasMsg())
		{
			_recvBuff.pop(frontMsg()->dataLength);
		}
	}

	int SendDataReal()
	{
		resetDTSend();
		return _sendBuff.write2socket(_sockfd);
	}
	//发送数据
	int SendData(std::shared_ptr<DataHeader> header)
	{
		// int ret = 1;
		// if (header)
		// {
		// 	int nSendLen = header->dataLength;
		// 	const char * pSendData = (const char*)header.get();
		// 	while (_lastSendPos + nSendLen >= SEND_BUFF_SZIE)
		// 	{
		// 		int nCopyLen = SEND_BUFF_SZIE - _lastSendPos;
		// 		memcpy(_szSendBuf + _lastSendPos, pSendData, nCopyLen);
		// 		ret = send(_sockfd, _szSendBuf, SEND_BUFF_SZIE, 0);
		// 		if (SOCKET_ERROR == ret)
		// 		{
		// 			return ret;
		// 		}
		// 		pSendData += nCopyLen;
		// 		nSendLen -= nCopyLen;
		// 		_lastSendPos = 0;
		// 		resetDTSend();
		// 	}
		// 	memcpy(_szSendBuf + _lastSendPos, pSendData, nSendLen);
		// 	_lastSendPos += nSendLen;
		// }
		// else
		// {
		// 	CELLLog::Info("server send head is null\n");
		// }
		// return ret;
		int ret = SOCKET_ERROR;
		if (_sendBuff.push((const char*)header.get(), header->dataLength))
		{
			ret = header->dataLength;
		}
		return ret;
		
	}
	void resetDTHeart()
	{
		_dtHeart = 0;
	}
	void resetDTSend()
	{
		_dtSend = 0;
	}
	bool checkHeart(time_t dt)
	{
		_dtHeart += dt;
		if (_dtHeart >= CLIENT_HEART_DEAD_TIME)
		{
			// CELLLog::Info("checkHeart dead:s=%d,time=%d\n", _sockfd, _dtHeart);
			return true;
		}
		return false;
	}
	void checkSend(time_t dt)
	{
		_dtSend += dt;
		if (_dtSend >= CLIENT_SEND_BUFF_TIME)
		{
			// CELLLog::Info("checkSend:s=%d,time=%d\n", _sockfd, _dtSend);
			SendDataReal();
			resetDTSend();
			
		}
	}

private:
	// socket fd_set  file desc set
	SOCKET _sockfd;
	//第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SZIE];
	//消息缓冲区的数据尾部位置
	int _lastPos;
	CELLBuffer _recvBuff;
	CELLBuffer _sendBuff;
	time_t _dtHeart;
	time_t _dtSend;
	bool _isFull;
};
#endif