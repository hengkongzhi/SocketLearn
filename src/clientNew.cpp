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
#include "CELLConfig.hpp"
#include "CELLThread.hpp"
#include <vector>
using namespace std;

const char* strIP = "127.0.0.1";
uint16_t nPort = 4567;
int nThread = 1;
int nClient = 1;
int nMsg = 1;
int nSendSleep = 1;
int nSendBuffSize = SEND_BUFF_SZIE;
int nRecvBuffSize = RECV_BUFF_SZIE;

// const int cCount = 10000;
// const int tCount = 8;
atomic_int sendCount(0);
atomic_int readyCount(0);
atomic_int nConnect(0);
// EasyTcpClient* client[nClient];

void workThread(CELLThread* pThread, int id)
{
    CELLLOG_Info("Thread<%d>,start.", id);
    vector<EasyTcpClient*> client(nClient);
    int begin = 0;
    int end = nClient;
    for (int i = begin; i < end; i++)
    {
        if (!pThread->isRun())
        {
            return;
        }
        client[i] = new EasyTcpClient();
        CELLThread::Sleep(0);
    }
    for (int i = begin; i < end; i++)
    {
        if (!pThread->isRun())
        {
            break;
        }
        if (INVALID_SOCKET == client[i]->InitSocket(nSendBuffSize, nRecvBuffSize))
        {
            break;
        }
        if (SOCKET_ERROR == client[i]->Connect(strIP, nPort))
        {
            break;
        }
        nConnect++;
        CELLThread::Sleep(0);
    }
    CELLLOG_Info("Thread<%d>, Connect<begin=%d, end=%d, nConnect=%d>", id, begin, end, (int)nConnect);
    
    readyCount++;
    while (readyCount < nThread && pThread->isRun())
    {
        CELLThread::Sleep(10);
    }
    
    shared_ptr<Login> login = make_shared<Login>();
    strcpy(login->userName, "yc");
    strcpy(login->PassWord, "yc123");
    while (pThread->isRun())
    {
        for (int i = begin; i < end; i++)
        {
            if (SOCKET_ERROR != client[i]->SendData(login))
            {
                sendCount++;
            }
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

int main(int argc, char* args[])
{
    CELLLog::Instance().SetLogPath("/root/LearnSocket/logs/Client", "w", false);
    CELLConfig::Instance().Init(argc, args);
    strIP = CELLConfig::Instance().getStr("strIP", "127.0.0.1");
    nPort = CELLConfig::Instance().getInt("nPort", 4567);
    nThread = CELLConfig::Instance().getInt("nThread", 1);
    nClient = CELLConfig::Instance().getInt("nClient", 10000);
    nMsg = CELLConfig::Instance().getInt("nMsg", 10);
    nSendSleep = CELLConfig::Instance().getInt("nSendSleep", 100);
    nSendBuffSize = CELLConfig::Instance().getInt("nSendBuffSize", SEND_BUFF_SZIE);
    nRecvBuffSize = CELLConfig::Instance().getInt("nRecvBuffSize", RECV_BUFF_SZIE);

    CELLThread tCmd;
    tCmd.Start(nullptr, [](CELLThread* pThread){
        while (pThread->isRun())
        {
            char cmdBuf[256] = {};
            scanf("%s", cmdBuf);
            if (0 == strcmp(cmdBuf, "exit"))
            {
                CELLLOG_Info("退出cmdThread线程");
                pThread->Exit();
            }
            else
            {
                CELLLOG_Info("不支持的命令。");
            }
        }
    });
    vector<CELLThread*> threads;
    for (int n = 0; n < nThread; n++)
    {
        CELLThread* t = new CELLThread();
        t->Start(nullptr, [n](CELLThread* pThread){
            workThread(pThread, n + 1);
        });
        threads.push_back(t);
    }
    
    CELLTimestamp tTime;
    while (tCmd.isRun())
    {
        auto t = tTime.getElapsedSecond();
        if (t >= 1.0)
        {
            CELLLOG_Info("thread<%d>,clients<%d>,connect<%d>,time<%lf>,send<%d>", nThread, nClient, (int)nConnect, t, (int)sendCount);
            tTime.update();
            sendCount = 0;
        }
        CELLThread::Sleep(1);
    }
    for (auto t : threads)
    {
        t->Close();
        delete t;
    }
    CELLLOG_Info("..已退出..");
    return 0;

}
