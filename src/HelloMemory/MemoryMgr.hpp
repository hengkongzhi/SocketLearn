#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_
#include <stdlib.h>
class MemoryAlloc;
class MemoryBlock
{
public:
    //所属大内存块（池）
    MemoryAlloc* pAlloc;
    //下一块的位置
    MemoryBlock* pNext;
    int nID;
    //引用次数
    int nRef;
    //是否在内存池中
    bool bPool;
private:
    //预留 
    char c1;
    char c2;
    char c3;
};

class MemoryAlloc
{
public:
    MemoryAlloc()
    {
        _pBuf = nullptr;
        _pHeader = nullptr;
        _nSize = 0;
        _nBlockSize = 0;

    }
    ~MemoryAlloc()
    {

    }
    void initMemory()
    {
        assert(nullptr == _pBuf);
        if (_pBuf)
        {
            return;
        }
        size_t bufSize = _nSize * _nBlockSize;
        _pBuf = (char*) malloc(bufSize);
        _pHeader = (MemoryBlock*) _pBuf;
        _pHeader->bPool = true;
        _pHeader->nID = 0;
        _pHeader->nRef = 0;
        _pHeader->pAlloc = this;
        _pHeader->pNext = nullptr;
        MemoryBlock* pTemp1 = _pHeader;
        for (size_t i = 1; i < _nBlockSize; i++)
        {
            MemoryBlock* pTemp2 = (MemoryBlock*) (_pBuf + (i * _nSize));
            pTemp2->bPool = true;
            pTemp2->nID = i;
            pTemp2->nRef = 0;
            pTemp2->pAlloc = this;
            pTemp1->pNext = pTemp2;
            pTemp1 = pTemp2;
        }

    }

private:
    //内存池地址
    char* _pBuf;
    //头部内存单元
    MemoryBlock* _pHeader;
    //内存单元的大小
    size_t _nSize;
    //内存单元的数量
    size_t _nBlockSize;
};
class MemoryMgr
{
private:
    MemoryMgr()
    {

    }
    ~MemoryMgr()
    {

    }
public:

    static MemoryMgr& Instance()
    {
        static MemoryMgr mgr;
        return mgr;
    }
    void* allocMem(size_t nSize)
    {
        return malloc(nSize);
    }
    void freeMem(void* p)
    {
        free(p);
    }
private:

};





#endif