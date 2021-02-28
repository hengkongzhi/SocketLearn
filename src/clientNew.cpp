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
         printf("退出cmdThread线程\n");
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
         printf("不支持的命令。\n");
        }
    }
};
const int cCount = 80;
const int tCount = 4;
atomic_int sendCount(0);
atomic_int readyCount(0);
EasyTcpClient* client[cCount];

void sendThread(int id)
{
    printf("Thread<%d>,start.\n", id);
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
    printf("Thread<%d>, Connect<begin=%d, end=%d>\n", id, begin, end);
    
    readyCount++;
    while (readyCount < tCount)
    {
        std::chrono::milliseconds t(10);
        std::this_thread::sleep_for(t);
    }
    

    Login login[10];
    for (int i = 0; i < 10; i++)
    {
        strcpy(login[i].userName, "yc");
        strcpy(login[i].PassWord, "yc123");
    }
    //strcpy(login.userName, "yc");
    //strcpy(login.PassWord, "yc123");
    const int nLen = sizeof(login); 
    while(g_bRun)
    {
        for (int i = begin; i < end; i++)
        {
            if (SOCKET_ERROR != client[i]->SendData(login, nLen))
            {
                sendCount++;
            }
            // else
            // {
            //     printf("fuck client\n");
            // }

            client[i]->OnRun();
        }
       
    }
    for (int i = begin; i < end; i++)
    {
        client[i]->Close();
        delete client[i];
    }
    printf("Thread<%d>,exit.\n", id);
};

int main()
{
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
            printf("thread<%d>,clients<%d>,time<%lf>,send<%d>\n", tCount, cCount, t, (int)(sendCount / t));
            tTime.update();
            sendCount = 0;
        }
        sleep(1);
    }
    
   // client1.Close();
    return 0;

}
