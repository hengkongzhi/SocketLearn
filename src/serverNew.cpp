#include <stdio.h>
#include <string>
#include <iostream>
#include "Alloctor.h"
#include "EasyTcpServer.hpp"
#include <signal.h>
#include "CELLConfig.hpp"
class MyServer : public EasyTcpServer
{
public:
    MyServer()
    {
        _bSendBack = CELLConfig::Instance().hasKey("-sendback");
        _bSendFull = CELLConfig::Instance().hasKey("-sendfull");
        _bCheckMsgID = CELLConfig::Instance().hasKey("-checkMsgID");
    }
    virtual void OnNetMsg(CellServer* pCellServer, ClientSocketPtr& pClient, DataHeader* header)
	{
        switch (header->cmd)
		{
			case CMD_LOGIN:
			{
				pClient->resetDTHeart();
				Login* login = (Login*)header;

                if (_bCheckMsgID)
                {
                    if (login->msgID != pClient->nRecvMsgID)
                    {
                        CELLLOG_Error("OnNetMsg socked<%d> msgID<%d> _nRecvMsgID<%d> %d", pClient->sockfd(), login->msgID,
                        pClient->nRecvMsgID, login->msgID - pClient->nRecvMsgID);
                    }
                    ++pClient->nRecvMsgID;
                }
                if (_bSendBack)
                {
                    std::shared_ptr<LoginResult> ret = std::make_shared<LoginResult>();
                    ret->msgID = pClient->nSendMsgID;
                    if (pClient->SendData(ret) == SOCKET_ERROR)
				    {
                        if (_bSendFull)
                        {
                            CELLLOG_Warring("socked<%d> Send full", pClient->sockfd());
                        }
				    }
                    else
                    {
                        ++pClient->nSendMsgID;
                    }
                }

			}
			break;
			case CMD_LOGOUT:
			{
				CELLRecvMsgStream r(header);
                auto n1 = r.ReadInt8();
                auto n2 = r.ReadInt16();
                auto n3 = r.ReadInt32();
                auto n4 = r.ReadFloat();
                auto n5 = r.ReadDouble();
                uint32_t n = 0;
                r.onlyRead(n);
                char name[32] = {0};
                r.ReadArray(name, 32);
                char pwd[32] = {0};
                r.ReadArray(pwd, 32);
                int data[10] = {0};
                r.ReadArray(data, 10);

				CELLSendMsgStream s;
    			s.setNetCmd(CMD_LOGOUT_RESULT);
    			s.WriteInt8(5);
    			s.WriteInt16(5);
    			s.WriteInt32(5.0f);
    			s.WriteFloat(5.0f);
    			s.WriteDouble(5.0f);
    			const char* str = "helloworld";
    			s.WriteArray(str, strlen(str));
    			char a[] = "asdsa";
    			s.WriteArray(a, strlen(a));
    			int b[] = {1, 2, 3, 4, 5};
    			s.WriteArray(b, 5);
    			s.finsh();
				pClient->SendData(s.data(), s.length());
			}
			break;
			case CMD_C2S_HEART:
			{
				pClient->resetDTHeart();
				std::shared_ptr<s2c_Heart> ret = std::make_shared<s2c_Heart>();
				pClient->SendData(ret);

			}
			default:
			{
				CELLLOG_Info("<socket=%d>收到未定义消息,数据长度：%d", pClient->sockfd(), header->dataLength);
			}
			break;
		}
		_msgCount++;
	}
private:
    bool _bSendBack = false;
    bool _bSendFull = false;
    bool _bCheckMsgID = false;
};

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
    MyServer server;
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
