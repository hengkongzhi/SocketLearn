#ifndef _CELL_STREAM_H
#define _CELL_STREAM_H
class CELLStream
{
public:
    CELLStream(char* pData, int nSize, bool bDel = false)
    {
        _nSize = nSize;
        _pBuff = pData;
        _bDel = bDel;
    }
    CELLStream(int nSize = 1024)
    {
        _nSize = nSize;
        _pBuff = new char[_nSize];
    }
    ~CELLStream()
    {
        if (_pBuff && _bDel)
        {
            delete []_pBuff;
            _pBuff = nullptr;
        }
    }
    template<typename T>
    bool Write(T n)
    {
        size_t nLen = sizeof(T);
        if (_nWritePos + nLen <= _nSize)
        {
            memcpy(_pBuff + _nWritePos, &n, nLen);
            _nWritePos += nLen;
            return true;
        }
        return false;
    }
    template<typename T>
    bool WriteArray(T* pData, uint32_t len)
    {
        auto nLen = sizeof(T) * len;
        if (_nWritePos + nLen + sizeof(uint32_t) <= _nSize)
        {
            WriteInt32(len);
            memcpy(_pBuff + _nWritePos, pData, nLen);
            _nWritePos += nLen;
            return true;
        }
        return false;
    }


    bool WriteInt8(int8_t n)
    {
        return Write(n);
    }
    bool WriteInt16(int16_t n)
    {
        return Write(n);
    }
    bool WriteInt32(int32_t n)
    {
        return Write(n);
    }
    bool WriteFloat(float n)
    {
        return Write(n);
    }
    bool WriteDouble(double n)
    {
        return Write(n);
    }
private:
    char* _pBuff = nullptr;
    int _nSize = 0;
    int _nWritePos = 0;
    int _nReadPos = 0;
    bool _bDel = true;
};
#endif