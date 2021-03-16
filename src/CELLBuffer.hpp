#ifndef _CELL_BUFFER_HPP_
#define _CELL_BUFFER_HPP_
class CELLBuffer
{
public:
    CELLBuffer(int nSize = 8192)
    {
        _nSize = nSize;
        _pBuff = new char[_nSize];
    }
    ~CELLBuffer()
    {
        if (_pBuff)
        {
            delete[] _pBuff;
            _pBuff = nullptr;
        }
    }
    char* data()
    {
        return _pBuff;
    }
    bool push(const char* pData, int nLen)
    {
        if (_nLast + nLen >_nSize)
        {
            int n = _nLast + nLen - _nSize;
            if (n < 1024)
            {
                n = 1024;
            }
            char* buff = new char[_nSize + n];
            memcpy(buff, _pBuff, _nLast);
            delete[] _pBuff;
            _pBuff = buff;
            _nSize = _nSize + n;
        }
        if (_nLast + nLen <=_nSize)
        {
            memcpy(_pBuff + _nLast, pData, nLen);
            _nLast += nLen;
            if (_nLast == _nSize)
            {
                ++_BuffFullCount;
            }
            return true;
        }
        else
        {
            ++_BuffFullCount;
        }
        return false;
    }
    void pop(int nLen)
    {
        int n = _nLast - nLen;
        if (n > 0)
        {
            memcpy(_pBuff, _pBuff + nLen, n);
        }
        _nLast = n;
    }
    int write2socket(SOCKET sockfd)
    {
        int ret = 0;
        if (_nLast > 0 && sockfd != INVALID_SOCKET)
        {
            ret = send(sockfd, _pBuff, _nLast, 0);
            _nLast = 0;
            _BuffFullCount = 0;
        }
        return ret;
    }
    int read4socket(SOCKET sockfd)
    {
        if (_nSize - _nLast > 0)
        {
            char* szRecv = _pBuff + _nLast;
            int nLen = (int)recv(sockfd, szRecv, _nSize - _nLast, 0);
            if (nLen <= 0)
            {
                return -1;
            }
            _nLast += nLen;
            return nLen;
        }
        return 0;
    }
    bool hasMsg()
    {
	    //判断消息缓冲区的数据长度大于消息长度
	    if (_nLast >= sizeof(DataHeader))
	    {
            //这时就可以知道当前消息的长度
	        DataHeader* header = (DataHeader*)_pBuff;
            if (_nLast >= header->dataLength)
            {
                return true;
            }  
	    }
        return false;
    }

private:
    char* _pBuff = nullptr;
    int _nLast = 0;
    int _nSize;
    int _BuffFullCount = 0;
};
#endif