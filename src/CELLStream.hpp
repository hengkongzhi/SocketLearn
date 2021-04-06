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
public:
    inline bool canRead(int n)
    {
        return _nSize - _nReadPos >= n;
    }
    inline bool canWrite(int n)
    {
        return _nSize - _nWritePos >= n;
    }
    inline void push(int n)
    {
        _nWritePos += n;
    }
    inline void pop(int n)
    {
        _nReadPos += n;
    }
    template<typename T>
    uint32_t ReadArray(T* pArr, uint32_t len)
    {
        uint32_t len1 = 0;
        Read(len1, false);
        if (len1 < len)
        {
            auto tSize = len1 * sizeof(T);
            if (canRead(tSize + sizeof(uint32_t)))
            {
                pop(sizeof(uint32_t));
                memcpy(pArr, _pBuff + _nReadPos, tSize);
                pop(tSize);
                return len1;
            }
        }
        return 0;
    }
    template<typename T>
    bool onlyRead(T& n)
    {
        return Read(n, false);
    }
    int8_t ReadInt8(int8_t n = 0)
    {
        Read(n);
        return n;
    }
    int16_t ReadInt16(int16_t n = 0)
    {
        Read(n);
        return n;
    }
    int32_t ReadInt32(int32_t n = 0)
    {
        Read(n);
        return n;
    }
    float ReadFloat(float n = 0)
    {
        Read(n);
        return n;
    }
    double ReadDouble(double n = 0)
    {
        Read(n);
        return n;
    }
    template<typename T>
    bool Read(T& n, bool bOffset = true)
    {
        auto nLen = sizeof(T);
        if (canRead(nLen))
        {
            memcpy(&n, _pBuff + _nReadPos, nLen);
            if (bOffset)
            {
                pop(nLen);
            }
            return true;
        }
        return false;
    }

    template<typename T>
    bool Write(T n)
    {
        size_t nLen = sizeof(T);
        if (canWrite(nLen))
        {
            memcpy(_pBuff + _nWritePos, &n, nLen);
            push(nLen);
            return true;
        }
        return false;
    }
    template<typename T>
    bool WriteArray(T* pData, uint32_t len)
    {
        auto nLen = sizeof(T) * len;
        if (canWrite(nLen + sizeof(uint32_t)))
        {
            WriteInt32(len);
            memcpy(_pBuff + _nWritePos, pData, nLen);
            push(nLen);
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