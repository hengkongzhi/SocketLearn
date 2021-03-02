#ifndef _CELL_THREAD_HPP_
#define _CELL_THREAD_HPP_
#include "CELLSemaphore.hpp"
#include <functional>
class CELLThread
{
private:
    typedef std::function<void(CELLThread*)> EventCall;
public:
    void Start(EventCall onCreate = nullptr, 
               EventCall onRun = nullptr, 
               EventCall onDestroy = nullptr)
    {

        if (!_isRun)
        {
            if (onCreate)
            {
                _onCreate = onCreate;
            }
            if (onRun)
            {
                _onRun = onRun;
            }
            if (onDestroy)
            {
                _onDestroy = onDestroy;
            }
            _isRun = true;
            std::thread t(std::mem_fn(&CELLThread::OnWork), this);
            t.detach();

        }
    }   
    void Close()
    {
        if (_isRun)
        {
            _isRun = false;
            _sem.wait();
        }
    }
    bool isRun()
    {
        return _isRun;
    }
protected:
    void OnWork()
    {
        if (_onCreate)
        {
            _onCreate(this);
        }
        if (_onRun)
        {
            _onRun(this);
        }
        if (_onDestroy)
        {
            _onDestroy(this);
        }
        _sem.wakeUp();
    }
private:
    EventCall _onCreate;
    EventCall _onRun;
    EventCall _onDestroy;
    bool _isRun = false;
    CELLSemaphore _sem;
};
#endif