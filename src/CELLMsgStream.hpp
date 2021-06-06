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
        pop(header->dataLength);
    }
public:
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
    void finsh()
    {
        int pos = getWritePos();
        setWritePos(0);
        Write<uint16_t>(pos);
        setWritePos(pos);
    }
};
#endif