#ifndef _CELL_TASK_H_
#define _CELL_TASK_H_
#include <thread>
#include <list>
#include <mutex>
class CellTask
{
public:
    CellTask()
    {

    }
    virtual ~CellTask()
    {

    }
    virtual void doTask()
    {

    }
private:

};
class CellTaskServer
{
private:
    std::list<std::shared_ptr<CellTask>> _tasks;
    std::list<std::shared_ptr<CellTask>> _tasksBuf;
    std::mutex _mutex;
public:
    CellTaskServer()
    {

    }
    ~CellTaskServer()
    {

    }
    void addTask(std::shared_ptr<CellTask> task)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _tasksBuf.push_back(task);
    }
    void Start()
    {
        std::thread t(std::mem_fn(&CellTaskServer::OnRun), this);
        t.detach();
    }
    void OnRun()
    {
        while (true)
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
                pTask->doTask();
            }
            _tasks.clear();
        }


    }
};

#endif