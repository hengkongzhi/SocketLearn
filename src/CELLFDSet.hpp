#ifndef _CELL_FDSet_HPP_
#define _CELL_FDSet_HPP_
#include "CELL.hpp"
#include<unistd.h> //uni std
#include<arpa/inet.h>
#include <string.h>
class CELLFDSet
{
public:
    CELLFDSet()
    {
        int nSocketNum = 10240;
        _nfdSize = nSocketNum / (8 * sizeof(char));

        _pfdSet = (fd_set*)new char[_nfdSize];
        memset(_pfdSet, 0, _nfdSize);
    }
    ~CELLFDSet()
    {
        if (_pfdSet)
        {
            delete [] _pfdSet;
            _pfdSet = nullptr;
        }
    }
    void add(SOCKET s)
    {
        FD_SET(s, _pfdSet);
    }
    void del(SOCKET s)
    {
        FD_CLR(s, _pfdSet);
    }
    void zero()
    {
        memset(_pfdSet, 0, _nfdSize);
    }
    void has(SOCKET s)
    {
        FD_ISSET(s, _pfdSet);
    }
private:
    fd_set* _pfdSet = nullptr;
    size_t _nfdSize = 0;
};
#endif