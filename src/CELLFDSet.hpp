#ifndef _CELL_FDSet_HPP_
#define _CELL_FDSet_HPP_
#include "CELL.hpp"
#include<unistd.h> //uni std
#include<arpa/inet.h>
#include <string.h>
#include "CELLLog.hpp"
// #define CELL_MAX_FD 1300
class CELLFDSet
{
public:
    CELLFDSet()
    {

    }
    ~CELLFDSet()
    {
        destroy();
    }
    void create(int maxFds)
    {
        if (maxFds < 65535)
        {
            maxFds = 65535;
        }
        CELL_MAX_FD = maxFds;
        int nSocketNum = maxFds;
        _nfdSize = nSocketNum / (8 * sizeof(char)) + 1;

        _pfdSet = (fd_set*)new char[_nfdSize];
        memset(_pfdSet, 0, _nfdSize);
    }
    void destroy()
    {
        if (_pfdSet)
        {
            delete [] _pfdSet;
            _pfdSet = nullptr;
        }
    }
    inline void add(SOCKET s)
    {
        if (s < CELL_MAX_FD)
        {
            FD_SET(s, _pfdSet);
        }
        else
        {
            CELLLOG_Error("CELLFDSet::add sock<%d>, CELL_MAX_FD<%d>", (int)s, CELL_MAX_FD);
        }
        
    }
    inline void del(SOCKET s)
    {
        FD_CLR(s, _pfdSet);
    }
    inline void zero()
    {
        memset(_pfdSet, 0, _nfdSize);
    }
    inline bool has(SOCKET s)
    {
        return FD_ISSET(s, _pfdSet);
    }
    inline fd_set* fdset()
    {
        return _pfdSet;
    }
    inline void copy(CELLFDSet& set)
    {
        memcpy(_pfdSet, set.fdset(), _nfdSize);
    }
private:
    fd_set* _pfdSet = nullptr;
    size_t _nfdSize = 0;
    int CELL_MAX_FD;
};
#endif