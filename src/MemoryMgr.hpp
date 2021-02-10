#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <mutex>

#define xPrintf(...) printf(__VA_ARGS__)

#define MAX_MEMORY_SIZE 128
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
        // xPrintf("MemoryAlloc\n");

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
        std::lock_guard<std::mutex> lg(_mutex);
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
        //xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
        return ((char*) pReturn + sizeof(MemoryBlock));
    }
    void freeMemory(void* pMem)
    {
        MemoryBlock* pBlock = (MemoryBlock*) ((char*) pMem - sizeof(MemoryBlock));
        assert(1 == pBlock->nRef);
        if (pBlock->bPool)
        {
            std::lock_guard<std::mutex> lg(_mutex);
            if (--pBlock->nRef != 0)
            {
                return;
            }
            pBlock->pNext = _pHeader;
            _pHeader = pBlock;
        }
        else
        {
            if (--pBlock->nRef != 0)
            {
                return;
            }
            free(pBlock);
        }
    }
    void initMemory()
    {
        // xPrintf("initMemory:_nsize=%d, _nBlockSize=%d\n", _nSize, _nBlockSize);
        assert(nullptr == _pBuf);
        if (_pBuf)
        {
            return;
        }
        size_t bufSize = (_nSize + sizeof(MemoryBlock)) * _nBlockSize;
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
            MemoryBlock* pTemp2 = (MemoryBlock*) (_pBuf + i * (_nSize + sizeof(MemoryBlock)));
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
    std::mutex _mutex; 
};
template<size_t nSize, size_t nBlockSize>
class MemoryAlloctor : public MemoryAlloc
{
public:
    MemoryAlloctor()
    {
        const size_t n = sizeof(void*);
        _nSize = (nSize / n) * n + ((nSize % n) ? n : 0);
        _nBlockSize = nBlockSize;
    }
};
class MemoryMgr
{
private:
    MemoryMgr()
    {
        init_szAlloc(0, 64, &_mem64);
        init_szAlloc(65, 128, &_mem128);
        // init_szAlloc(129, 256, &_mem256);
        // init_szAlloc(257, 512, &_mem512);
        // init_szAlloc(513, 1024, &_mem1024);
        // xPrintf("MemoryMgr\n");
    }
    ~MemoryMgr()
    {

    }
    void init_szAlloc(int nBegin, int nEnd, MemoryAlloc* pMemA)
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
            //xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
            return (char*) pReturn + sizeof(MemoryBlock);
        }
    }
    void freeMem(void* pMem)
    {
        MemoryBlock* pBlock = (MemoryBlock*) ((char*)pMem - sizeof(MemoryBlock));
        //xPrintf("freeMem: %llx, id=%d\n", pBlock, pBlock->nID);
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
        MemoryBlock* pBlock = (MemoryBlock*) ((char*)pMem - sizeof(MemoryBlock));
        ++pBlock->nRef;
    }
private:
    MemoryAlloctor<64, 1000> _mem64;
    MemoryAlloctor<128, 1000> _mem128;
    // MemoryAlloctor<256, 100000> _mem256;
    // MemoryAlloctor<512, 100000> _mem512;
    // MemoryAlloctor<1024, 100000> _mem1024;
    MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE + 1];

};





#endif