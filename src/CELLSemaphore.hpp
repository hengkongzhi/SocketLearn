#ifndef _CELL_SEMAPHORE_HPP_
#define _CELL_SEMAPHORE_HPP_
#include <chrono>
#include <thread>
class CELLSemaphore
{
public:
    void wait()
    {
        _isWaitExit = true;
        while (_isWaitExit)
        {
            std::chrono::milliseconds t(1);
            std::this_thread::sleep_for(t);
        }

    }
    void wakeUp()
    {
        if (_isWaitExit)
        {
            _isWaitExit = false;
        }
        else
        {
            printf("CELLSemaphore wakeUp error\n");
        }
        
    }

private:
    bool _isWaitExit = false;
};
#endif