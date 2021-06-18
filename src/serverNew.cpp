#include <stdio.h>
#include <string>
#include <iostream>
#include "Alloctor.h"
#include "EasyTcpServer.hpp"
#include <signal.h>
#include "CELLConfig.hpp"

// bool g_bRun1 = true;
// void cmdThread()
// {
//     while (true)
//     {
//         char cmdBuf[256] = {};
//         scanf("%s", cmdBuf);
//         if (0 == strcmp(cmdBuf, "exit"))
//         {
//          g_bRun1 = false;
//          CELLLOG_Info("退出cmdThread线程");
//          break;
//         }
//         else
//         {
//          CELLLOG_Info("不支持的命令。");
//         }
//     }
// }
const char* argToStr(int argc, char* args[], int index, const char* def, const char* argName)
{
    if (argc <= index)
    {
        CELLLOG_Error("argToStr, index=%d, argc=%d, argName=%s", index, argc, argName);
    }
    else
    {
        def = args[index];
    }
    CELLLOG_Info("%s=%s", argName, def);
    return def;
}
int argToInt(int argc, char* args[], int index, int def, const char* argName)
{
    if (argc <= index)
    {
        CELLLOG_Error("argToStr, index=%d, argc=%d, argName=%s", index, argc, argName);
    }
    else
    {
        def = atoi(args[index]);
    }
    CELLLOG_Info("%s=%d", argName, def);
    return def;
}
int main(int argc, char* args[])
{
    CELLLog::Instance().SetLogPath("/root/LearnSocket/logs/Server", "w");
    CELLConfig::Instance().Init(argc, args);
    const char* strIP = CELLConfig::Instance().getStr("strIP", "any");
    uint16_t nPort = CELLConfig::Instance().getInt("nPort", 4567);
    int nThread = CELLConfig::Instance().getInt("nThread", 1);
    int nClient = CELLConfig::Instance().getInt("nClient", 1);
    if (CELLConfig::Instance().hasKey("-p"))
    {
        CELLLOG_Info("hasKey -p");
    }
    // const char* strIP = argToStr(argc, args, 1, "any", "strIP");
    // uint16_t nPort = argToInt(argc, args, 2, 4567, "nPort");
    // int nThread = argToInt(argc, args, 3, 1, "nThread");
    // int nClient = argToInt(argc, args, 4, 1, "nClient");

    if (strcmp(strIP, "any") == 0)
    {
        strIP = nullptr;
    }
    //SIGPIPE ignore
    struct sigaction act;
    act.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &act, NULL) == 0) 
    {
    }
    EasyTcpServer server;
    server.InitSocket();
    server.Bind(strIP, nPort);
    server.Listen(1000);
    server.Start(nThread);
    
    while (true)
    {
        char cmdBuf[256] = {};
        scanf("%s", cmdBuf);
        if (0 == strcmp(cmdBuf, "exit"))
        {
            server.Close();
            CELLLOG_Info("退出程序");
            break;
        }
        else
        {
            CELLLOG_Info("不支持的命令。");
        }   
    }
    CELLLOG_Info("已退出。");
    return 0;
}
