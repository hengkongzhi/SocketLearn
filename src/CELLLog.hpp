#ifndef _CELL_LOG_HPP_
#define _CELL_LOG_HPP_
#include "CELLTask.hpp"
#include <chrono>
#include <ctime>
#include <time.h>
#define TIME_AREA 8
class CELLLog
{
#if _DEBUG
    #ifndef CELLLOG_Debug
        #define CELLLOG_Debug(...) CELLLog::Debug(__VA_ARGS__)
    #endif
#else
    #ifndef CELLLOG_Debug
        #define CELLLOG_Debug(...)
    #endif
#endif
#define CELLLOG_Error(...) CELLLog::Error(__VA_ARGS__)
#define CELLLOG_Warring(...) CELLLog::Warring(__VA_ARGS__)
#define CELLLOG_Info(...) CELLLog::Info(__VA_ARGS__)
#define CELLLOG_PError(...) CELLLog::pError(__VA_ARGS__)
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
            Info("CELLLog fclose(_logFile)");
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
    static void pError(const char* pStr)
    {
        pError("%s", pStr);
    }
    template<typename ...Args>
    static void pError(const char* pFormat, Args ...args)
    {
        auto errCode = errno;
        Instance()._taskServer.addTask([=](){
            EchoReal(true, "pError ", pFormat, args...);
            EchoReal(true, "pError ", "errno<%d>, errmsg<%s>", errCode, strerror(errCode));
        });
    }


    static void Error(const char* pStr)
    {
        Error("%s", pStr);
    }
    template<typename ...Args>
    static void Error(const char* pFormat, Args ...args)
    {
        Echo("Error ", pFormat, args...);
    }
    static void Warring(const char* pStr)
    {
        Warring("%s", pStr);
    }
    template<typename ...Args>
    static void Warring(const char* pFormat, Args ...args)
    {
        Echo("Warring ", pFormat, args...);
    }
    static void Debug(const char* pStr)
    {
        Debug("%s", pStr);
    }
    template<typename ...Args>
    static void Debug(const char* pFormat, Args ...args)
    {
        Echo("Debug ", pFormat, args...);
    }

    static void Info(const char* pStr)
    {
        Info("%s", pStr);
    }
    template<typename ...Args>
    static void Info(const char* pFormat, Args ...args)
    {
        Echo("Info ", pFormat, args...);
    }
    template<typename ...Args>
    static void Echo(const char* type, const char* pFormat, Args ...args)
    {
        CELLLog* pLog = &Instance();
        pLog->_taskServer.addTask([=](){
            EchoReal(true, type, pFormat, args...);
        });
    }
    template<typename ...Args>
    static void EchoReal(bool br, const char* type, const char* pFormat, Args ...args)
    {
        CELLLog* pLog = &Instance();
        if (pLog->_logFile)
        {
            auto t = std::chrono::system_clock::now();
            auto tNow = std::chrono::system_clock::to_time_t(t);
            std::tm* now = std::gmtime(&tNow);
            if (type)
            {
                fprintf(pLog->_logFile, "%s", type);
            }
            fprintf(pLog->_logFile, "[%04d-%02d-%02d %02d:%02d:%02d]", now->tm_year + 1900, now->tm_mon + 1, 
                    now->tm_mday, now->tm_hour + TIME_AREA, now->tm_min, now->tm_sec);
            fprintf(pLog->_logFile, pFormat, args...);
            if (br)
            {
                fprintf(pLog->_logFile, "\n");
            }
            fflush(pLog->_logFile);
        }
        if (type)
        {
            printf("%s", type);
        }
        printf(pFormat, args...);
        if (br)
        {
            printf("\n");
        }
    }
    void SetLogPath(const char* logName, const char* mode, bool hasDate = true)
    {
        if (_logFile)
        {
            Info("CELLLog::SetLogPath _logFile != nullptr");
            fclose(_logFile);
            _logFile = nullptr;
        }
        static char logPath[256] = {0};
        if (hasDate)
        {
            auto t = std::chrono::system_clock::now();
            auto tNow = std::chrono::system_clock::to_time_t(t);
            std::tm* now = std::gmtime(&tNow);
            //std::tm* now = std::localtime(&tNow);
            sprintf(logPath, "%s[%d-%d-%d_%d-%d-%d].txt", logName, now->tm_year + 1900, now->tm_mon + 1, 
                now->tm_mday, now->tm_hour + TIME_AREA, now->tm_min, now->tm_sec);
        }
        else
        {
            sprintf(logPath, "%s.txt", logName);
        }

        _logFile = fopen(logPath, mode);
        if (_logFile)
        {
            Info("CELLLog::SetLogPath success, <%s,%s>", logPath, mode);
        }
        else
        {
            Info("CELLLog::SetLogPath failed, <%s,%s>", logPath, mode);
        }
    }
private:
    FILE* _logFile = nullptr;
    CellTaskServer _taskServer;
};
#endif