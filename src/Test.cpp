#include "EasyTcpClient.hpp"
#include "CELLThread.hpp"
#include "CELLStream.hpp"
class MyClient : public EasyTcpClient
{

};
int main()
{
    CELLStream s;
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