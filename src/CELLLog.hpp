#ifndef _CELL_LOG_HPP_
#define _CELL_LOG_HPP_
#include "CELLTask.hpp"
class CELLLog
{
private:
    CELLLog()
    {
        _taskServer.Start();
    }
    ~CELLLog()
    {
        _taskServer.Close();
        if (_logFile)
        {
            Info("CELLLog fclose(_logFile)\n");
            fclose(_logFile);
            _logFile = nullptr;
        }
    }
public:
    static CELLLog& Instance()
    {
        static CELLLog sLog;
        return sLog;
    }
    static void Info(const char* pStr)
    {
        CELLLog* pLog = &Instance();
        if (pLog->_logFile)
        {
            fprintf(pLog->_logFile, "%s", pStr);
            fflush(pLog->_logFile);
        }
        
    }
    template<typename ...Args>
    static void Info(const char* pFormat, Args ...args)
    {
        CELLLog* pLog = &Instance();
        if (pLog->_logFile)
        {
            fprintf(pLog->_logFile, pFormat, args...);
            fflush(pLog->_logFile);
        }
    }
    void SetLogPath(const char* logPath, const char* mode)
    {
        if (_logFile)
        {
            Info("CELLLog::SetLogPath _logFile != nullptr\n");
            fclose(_logFile);
            _logFile = nullptr;
        }
        _logFile = fopen(logPath, mode);
        if (_logFile)
        {
            Info("CELLLog::SetLogPath success, <%s,%s>\n", logPath, mode);
        }
        else
        {
            Info("CELLLog::SetLogPath failed, <%s,%s>\n", logPath, mode);
        }
    }
private:
    FILE* _logFile = nullptr;
    CellTaskServer _taskServer;
};
#endif