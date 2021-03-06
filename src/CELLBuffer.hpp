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
            if (ret <= 0)
            {
                CELLLOG_PError("write2socket error...");
                return SOCKET_ERROR;
            }
            if (ret == _nLast)
            {
                _nLast = 0;
            }
            else
            {
                _nLast -= ret;
                memcpy(_pBuff, _pBuff + ret, _nLast);
            }
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
                CELLLOG_PError("read4socket, nLen=%d", nLen);
                return SOCKET_ERROR;
            }
            _nLast += nLen;
            return nLen;
        }
        return 0;
    }
    bool hasMsg()
    {
	    //??????????????????????????????????????????????????????
	    if (_nLast >= sizeof(DataHeader))
	    {
            //??????????????????????????????????????????
	        DataHeader* header = (DataHeader*)_pBuff;
            if (_nLast >= header->dataLength)
            {
                return true;
            }  
	    }
        return false;
    }
    bool NeedWrite()
	{
		return _nLast > 0;
	}

private:
    char* _pBuff = nullptr;
    int _nLast = 0;
    int _nSize;
    int _BuffFullCount = 0;
};
#endif