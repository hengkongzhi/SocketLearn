#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_
#include <stdlib.h>
class MemoryBlock
{
public:
    MemoryBlock()
    {

    }
    ~MemoryBlock()
    {

    }
private:

};

class MemoryAlloc
{
public:
    MemoryAlloc()
    {

    }
    ~MemoryAlloc()
    {

    }

private:

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