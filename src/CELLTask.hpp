#ifndef _CELL_TASK_H_
#define _CELL_TASK_H_
#include <thread>
#include <list>
#include <mutex>
#include <functional>
#include "CELLSemaphore.hpp"
// class CellTask
// {
// public:
//     CellTask()
//     {

//     }
//     virtual ~CellTask()
//     {

//     }
//     virtual void doTask()
//     {

//     }
// private:

// };
class CellTaskServer
{
public:
    int serverId = -1; 
private:
    bool _isRun = false;
    typedef std::function<void()> CellTask;
    std::list<CellTask> _tasks;
    std::list<CellTask> _tasksBuf;
    std::mutex _mutex;
    CELLSemaphore _semaphore;
    
public:
    CellTaskServer()
    {

    }
    ~CellTaskServer()
    {

    }
    void addTask(CellTask task)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _tasksBuf.push_back(task);
    }
    void Start()
    {
        _isRun = true;
        std::thread t(std::mem_fn(&CellTaskServer::OnRun), this);
        t.detach();
    }
    void Close()
    {
        printf("CellTaskServer%d.Close\n", serverId);
        _isRun = false;
        _semaphore.wait();
    }
    void OnRun()
    {
        while (_isRun)
        {
            if (!_tasksBuf.empty())
            {
                std::lock_guard<std::mutex> lock(_mutex);
                for (auto pTask : _tasksBuf)
                {
                    _tasks.push_back(pTask);
                }
                _tasksBuf.clear();
            }
            if (_tasks.empty())
            {
                std::chrono::milliseconds t(1);
                std::this_thread::sleep_for(t);
                continue;
            }
            for (auto pTask : _tasks)
            {
                pTask();
            }
            _tasks.clear();
        }
        printf("CellTaskServer%d.OnRun\n", serverId);
        _semaphore.wakeUp();
    }
};

#endif