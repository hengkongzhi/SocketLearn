#include "EasyTcpClient.hpp"
#include "CELLThread.hpp"
#include "CELLMsgStream.hpp"
class MyClient : public EasyTcpClient
{
public:
    virtual void OnNetMsg(DataHeader* header)
    {
        switch (header->cmd)
		{
			case CMD_LOGIN_RESULT:
			{
                CELLRecvMsgStream r(header);
                r.getNetCmd();
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
				LoginResult* login = (LoginResult*)header;
				//CELLLog::Info("<socket=%d>收到服务端消息：CMD_LOGIN_RESULT,数据长度：%d\n", _sock, login->dataLength);
			}
			break;
			default:
			{
			}
            break;
		}
    }
};
int main()
{
    CELLSendMsgStream s;
    s.setNetCmd(CMD_LOGIN);
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



    MyClient client;
    client.Connect("192.168.0.115", 4567);
    while (client.isRun())
    {
        client.OnRun();
        CELLThread::Sleep(10);
    }
    return 0;
}