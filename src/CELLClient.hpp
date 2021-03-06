#ifndef _CELL_CLIENT_HPP_
#define _CELL_CLIENT_HPP_
#include "CELLBuffer.hpp"
#include "CELL.hpp"
#include"CELLObjectPool.hpp"
#include "CELLNetWork.hpp"
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
	int nRecvMsgID;
	int nSendMsgID;
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET, int sendBuff = SEND_BUFF_SZIE, int recvBuff = RECV_BUFF_SZIE) : _sendBuff(SEND_BUFF_SZIE), _recvBuff(RECV_BUFF_SZIE)
	{
		static int n = 1;
		id = n++;
		nRecvMsgID = 1;
		nSendMsgID = 1;
		_sockfd = sockfd;
		resetDTHeart();
		resetDTSend();
		_isFull = false;
	}
	~ClientSocket()
	{
		CELLLOG_Debug("s=%d ClientSocket%d.~ClientSocket", serverId, id);
		CELLNetWork::destroy(_sockfd);
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
	bool NeedWrite()
	{
		return _sendBuff.NeedWrite();
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
		// 	CELLLOG_Info("server send head is null");
		// }
		// return ret;
		return SendData((const char*)header.get(), header->dataLength);
	}
	int SendData(const char* pData, int len)
	{
		if (_sendBuff.push(pData, len))
		{
			return len;
		}
		return SOCKET_ERROR;
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
			// CELLLOG_Info("checkHeart dead:s=%d,time=%d", _sockfd, _dtHeart);
			return true;
		}
		return false;
	}
	void checkSend(time_t dt)
	{
		_dtSend += dt;
		if (_dtSend >= CLIENT_SEND_BUFF_TIME)
		{
			// CELLLOG_Info("checkSend:s=%d,time=%d", _sockfd, _dtSend);
			SendDataReal();
			resetDTSend();
			
		}
	}

private:
	// socket fd_set  file desc set
	SOCKET _sockfd;
	CELLBuffer _recvBuff;
	CELLBuffer _sendBuff;
	time_t _dtHeart;
	time_t _dtSend;
	bool _isFull;
};
#endif