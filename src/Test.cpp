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

    auto n1 = s.ReadInt8();
    auto n2 = s.ReadInt16();
    auto n3 = s.ReadInt32();
    auto n4 = s.ReadFloat();
    auto n5 = s.ReadDouble();
    uint32_t n = 0;
    s.onlyRead(n);
    char name[32] = {0};
    s.ReadArray(name, 32);
    char pwd[32] = {0};
    s.ReadArray(pwd, 32);
    int data[10] = {0};
    s.ReadArray(data, 10);

    MyClient client;
    client.Connect("192.168.0.115", 4567);
    while (client.isRun())
    {
        client.OnRun();
        CELLThread::Sleep(10);
    }
    return 0;
}