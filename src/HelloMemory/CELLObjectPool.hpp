#ifndef _CELLObjectPool_hpp_
#define _CELLObjectPool_hpp_
#include <stdlib.h>
#include <mutex>
#include <assert.h>

template<class Type, size_t nPoolSize>
class CELLObjectPool
{
public:
    CELLObjectPool()
    {
        initPool();
    }
    ~CELLObjectPool()
    {
        if (_pBuf)
        {
            delete[] _pBuf;
        }
    }
private:
class NodeHeader
{
    public:
        NodeHeader* pNext;
        int nID;
        char nRef;
        bool bPool;
    private:
        char c1;
        char c2;
    };
    void initPool()
    {
        assert(nullptr == _pBuf);
        if (_pBuf)
        {
            return;
        }
        size_t n = nPoolSize * (sizeof(Type) + sizeof(NodeHeader));
        _pBuf = new char[n];
        _pHeader = (NodeHeader*) _pBuf;
        _pHeader->bPool = true;
        _pHeader->nID = 0;
        _pHeader->nRef = 0;
        _pHeader->pNext = nullptr;
        NodeHeader* pTemp1 = _pHeader;
        for (size_t n = 1; n < nPoolSize; n++)
        {
            NodeHeader* pTemp2 = (NodeHeader*)(_pBuf + n * (sizeof(Type) + sizeof(NodeHeader)));
            pTemp2->bPool = true;
            pTemp2->nID = n;
            pTemp2->nRef = 0;
            pTemp2->pNext = nullptr;
            pTemp1->pNext = pTemp2;
            pTemp1 = pTemp2;
        }
    }
public:


    void* allocObjMemory(size_t nSize)
    {
        std::lock_guard<std::mutex> lg(_mutex);
        if (!_pBuf)
        {
            initPool();
        }
        NodeHeader* pReturn = nullptr;
        if (nullptr == _pHeader)
        {
            pReturn = (NodeHeader*) new char[(sizeof(Type) + sizeof(NodeHeader))];
            pReturn->bPool = false;
            pReturn->nID = -1;
            pReturn->nRef = 1;
            pReturn->pNext = nullptr;
        }
        else
        {
            pReturn = _pHeader;
            _pHeader = _pHeader->pNext;
            assert(0 == pReturn->nRef);
            pReturn->nRef = 1;
        }
        printf("allocObjMemory: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
        return ((char*) pReturn + sizeof(NodeHeader));
    }
    void freeObjMemory(void* pMem)
    {
        NodeHeader* pBlock = (NodeHeader*) ((char*) pMem - sizeof(NodeHeader));
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
            delete[] pBlock;
        }
    }

private:
    NodeHeader* _pHeader;
    char* _pBuf;
    std::mutex _mutex;


};


template<class Type, size_t nPoolSize>
class ObjectPoolBase
{
public:
    void* operator new(size_t nSize)
    {
        return objectPool().allocObjMemory(nSize);
    }
    void operator delete(void* p)
    {
        objectPool().freeObjMemory(p);
    }
    template<typename ...Args>
    static Type* createObject(Args ...args)
    {
        Type* obj = new Type(args...);
        return obj;
    }
    static void destroyObject(Type* obj)
    {
        delete obj;
    }
private:
    typedef CELLObjectPool<Type, nPoolSize> ClassTypePool;
    static ClassTypePool& objectPool()
    {
        static ClassTypePool sPool;
        return sPool;
    }
};



#endif