#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_
#include <stdlib.h>

#define MAX_MEMORY_SIZE 64
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
        if (_pBuf)
        {
            free(_pBuf);
        }
    }
    void* allocMemory(size_t nSize)
    {
        if (!_pBuf)
        {
            initMemory();
        }
        MemoryBlock* pReturn = nullptr;
        if (nullptr == _pHeader)
        {
            pReturn = (MemoryBlock*) malloc(nSize + sizeof(MemoryBlock));
            pReturn->bPool = false;
            pReturn->nID = -1;
            pReturn->nRef = 1;
            pReturn->pAlloc = nullptr;
            pReturn->pNext = nullptr;
        }
        else
        {
            pReturn = _pHeader;
            _pHeader = _pHeader->pNext;
            assert(0 == pReturn->nRef);
            pReturn->nRef = 1;
        }
        return ((char*) pReturn + sizeof(MemoryBlock));
    }
    void freeMemory(void* pMem)
    {
        MemoryBlock* pBlock = (MemoryBlock*) (char* pMem - sizeof(MemoryBlock));
        assert(1 == pBlock->nRef);
        if (--pBlock->nRef != 0)
        {
            return;
        }
        if (pBlock->bPool)
        {
            pBlock->pNext = _pHeader;
            _pHeader = pBlock;
        }
        else
        {
            free(pBlock);
        }
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
            pTemp2->pNext = nullptr;
            pTemp1->pNext = pTemp2;
            pTemp1 = pTemp2;
        }

    }

protected:
    //内存池地址
    char* _pBuf;
    //头部内存单元
    MemoryBlock* _pHeader;
    //内存单元的大小
    size_t _nSize;
    //内存单元的数量
    size_t _nBlockSize;
};
template<size_t nSize, size_t nBlockSize>
class MemoryAlloctor : public MemoryAlloc
{
public:
    MemoryAlloctor()
    {
        const size_t n = sizeof(void*);
        _nSize = (nSize / n) * n + (nSize % n) ? n : 0;
        _nBlockSize = nBlockSize;
    }
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
    void init(int nBegin, int nEnd, MemoryAlloc* pMemA)
    {
        for (int n = nBegin; n <= nEnd; n++)
        {
            _szAlloc[n] = pMemA;
        }
    }
public:

    static MemoryMgr& Instance()
    {
        static MemoryMgr mgr;
        return mgr;
    }
    void* allocMem(size_t nSize)
    {
        if (nSize <= MAX_MEMORY_SIZE)
        {
            return _szAlloc[nSize]->allocMemory(nSize);
        }
        else
        {
            MemoryBlock* pReturn = (MemoryBlock*) malloc(nSize + sizeof(MemoryBlock));
            pReturn->bPool = false;
            pReturn->nID = -1;
            pReturn->nRef = 1;
            pReturn->pAlloc = nullptr;
            pReturn->pNext = nullptr;
            return pReturn;
        }
    }
    void freeMem(void* pMem)
    {
        MemoryAlloc* pBlock = (MemoryAlloc*) ((char*)pMem - sizeof(MemoryBlock));
        if (pBlock->bPool)
        {
            pBlock->pAlloc->freeMemory(pMem);
        }
        else
        {
            if (--pBlock->nRef == 0)
            {
                free(pBlock);
            }
        }
        
    }
    void addRef(void* pMem)
    {
        MemoryAlloc* pBlock = (MemoryAlloc*) ((char*)pMem - sizeof(MemoryBlock));
        ++pBlock->nRef;
    }
private:
    MemoryAlloctor<64, 10> _mem64;
    MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE + 1];

};





#endif