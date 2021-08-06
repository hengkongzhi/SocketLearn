#ifndef _CELLNETWORK_hpp_
#define _CELLNETWORK_hpp_
#include <unistd.h> //uni std
#include <arpa/inet.h>
#include "CELLLog.hpp"

class CELLNetWork
{
public:
    static int destroy(int sockfd)
	{
		int ret = close(sockfd);
		if (ret < 0)
		{
			CELLLOG_PError("close sockfd<%d> failed...", sockfd);
		}
		return ret;
	}
};
#endif