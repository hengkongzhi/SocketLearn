#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <thread>
#include "EasyTcpClient.hpp"
#include <chrono>
#include <atomic>
#include "CELLTimestamp.hpp"
using namespace std;
bool g_bRun = true;
void cmdThread()
{
    while (true)
    {
        char cmdBuf[256] = {};
        scanf("%s", cmdBuf);
        if (0 == strcmp(cmdBuf, "exit"))
        {
//         client->Close();
         g_bRun = false;
         CELLLOG_Info("退出cmdThread线程");
         break;
        }
//        else if (0 == strcmp(cmdBuf, "login"))
//        {
//         Login login;
//         strcpy(login.userName,"yc");
//         strcpy(login.PassWord,"123");
//         client->SendData(&login);
//        }
//        else if (0 == strcmp(cmdBuf, "logout"))
//        {
//         Logout logout;
//         strcpy(logout.userName,"yc");
//         client->SendData(&logout);
//        
//        }
        else
        {
         CELLLOG_Info("不支持的命令。");
        }
    }
};
const int cCount = 10000;
const int tCount = 8;
atomic_int sendCount(0);
atomic_int readyCount(0);
EasyTcpClient* client[cCount];

void sendThread(int id)
{
    CELLLOG_Info("Thread<%d>,start.", id);
    int c = cCount / tCount;
    int begin = (id - 1) * c;
    int end = id * c;
    for (int i = begin; i < end; i++)
    {
        client[i] = new EasyTcpClient();
    }
    for (int i = begin; i < end; i++)
    {
        if (!g_bRun)
        {
            return;
        }
        client[i]->Connect("192.168.0.115", 4567);

    }
    CELLLOG_Info("Thread<%d>, Connect<begin=%d, end=%d>", id, begin, end);
    
    readyCount++;
    while (readyCount < tCount)
    {
        std::chrono::milliseconds t(10);
        std::this_thread::sleep_for(t);
    }
    

    // Login login[1];
    // shared_ptr<Login> login(new Login);
    shared_ptr<Login> login = make_shared<Login>();
    // for (int i = 0; i < 1; i++)
    // {
    //     strcpy(login[i].userName, "yc");
    //     strcpy(login[i].PassWord, "yc123");
    // }
    strcpy(login->userName, "yc");
    strcpy(login->PassWord, "yc123");
    // const int nLen = sizeof(login); 
    while(g_bRun)
    {
        for (int i = begin; i < end; i++)
        {
            if (SOCKET_ERROR != client[i]->SendData(login))
            {
                sendCount++;
            }
            // else
            // {
            //     CELLLOG_Info("fuck client");
            // }

            client[i]->OnRun();
            // std::chrono::microseconds t(1);
            // std::this_thread::sleep_for(t);
        }
        std::chrono::milliseconds t(500);
        std::this_thread::sleep_for(t);
       
    }
    for (int i = begin; i < end; i++)
    {
        client[i]->Close();
        delete client[i];
    }
    CELLLOG_Info("Thread<%d>,exit.", id);
};

int main()
{
    CELLLog::Instance().SetLogPath("/root/LearnSocket/logs/Client", "w");
    std::thread t1(cmdThread);
    t1.detach();
    for (int n = 0; n < tCount; n++)
    {
        std::thread t(sendThread, n + 1);
        t.detach();   
    }
    CELLTimestamp tTime;
    while (g_bRun)
    {
        auto t = tTime.getElapsedSecond();
        if (t >= 1.0)
        {
            CELLLOG_Info("thread<%d>,clients<%d>,time<%lf>,send<%d>", tCount, cCount, t, (int)sendCount);
            tTime.update();
            sendCount = 0;
        }
        sleep(1);
    }
    
   // client1.Close();
    return 0;

}
