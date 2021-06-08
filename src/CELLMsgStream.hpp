#ifndef _CELL_MSG_STREAM_H
#define _CELL_MSG_STREAM_H
#include "MessageHeader.hpp"
#include "CELLStream.hpp"
class CELLRecvMsgStream : public CELLStream
{
public:
    CELLRecvMsgStream(DataHeader* header)
        :CELLStream((char*)header, header->dataLength)
    {
        push(header->dataLength);
        ReadInt16();
        getNetCmd();
    }
    uint16_t getNetCmd()
    {
        uint16_t cmd = CMD_ERROR;
        Read<uint16_t>(cmd);
        return cmd;
    }
};
class CELLSendMsgStream : public CELLStream
{
public:
    CELLSendMsgStream(char* pData, int nSize, bool bDel = false)
        :CELLStream(pData, nSize, bDel)
    {
        Write<uint16_t>(0);
    }
    CELLSendMsgStream(int nSize = 1024)
        :CELLStream(nSize)
    {
        Write<uint16_t>(0);
    }
    void setNetCmd(uint16_t cmd)
    {
        Write<uint16_t>(cmd);
    }
    void finsh()
    {
        int pos = getWritePos();
        setWritePos(0);
        Write<uint16_t>(pos);
        setWritePos(pos);
    }
};
#endif