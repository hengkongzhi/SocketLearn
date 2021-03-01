#ifndef _CELL_SEMAPHORE_HPP_
#define _CELL_SEMAPHORE_HPP_
#include <chrono>
#include <thread>
#include <condition_variable>
class CELLSemaphore
{
public:
    void wait()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        if (--_wait < 0)
        {
            _cv.wait(lock, [this]()->bool{return _wakeup > 0;});
            _wakeup--;
        }

    }
    void wakeUp()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (++_wait <= 0)
        {
            _cv.notify_one();
            _wakeup++;
        }
        
    }

private:
    std::mutex _mutex;
    std::condition_variable _cv;
    int _wait = 0;
    int _wakeup = 0;
};
#endif