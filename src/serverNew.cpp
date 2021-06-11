#include <stdio.h>
#include <string>
#include <iostream>
#include "Alloctor.h"
#include "EasyTcpServer.hpp"
#include <signal.h>

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
//          CELLLog::Info("退出cmdThread线程\n");
//          break;
//         }
//         else
//         {
//          CELLLog::Info("不支持的命令。\n");
//         }
//     }
// }

int main()
{
    CELLLog::Instance().SetLogPath("/root/LearnSocket/logs/Server.txt", "w");
    //SIGPIPE ignore
    struct sigaction act;
    act.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &act, NULL) == 0) 
    {
    }
    EasyTcpServer server;
    server.InitSocket();
    server.Bind(nullptr, 4567);
    server.Listen(1000);
    server.Start(8);
    
    while (true)
    {
        char cmdBuf[256] = {};
        scanf("%s", cmdBuf);
        if (0 == strcmp(cmdBuf, "exit"))
        {
            server.Close();
            CELLLog::Info("退出程序\n");
            break;
        }
        else
        {
            CELLLog::Info("不支持的命令。\n");
        }   
    }
    CELLLog::Info("已退出。\n");
    return 0;
}
