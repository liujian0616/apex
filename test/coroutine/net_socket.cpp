#include <memory.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include "net_socket.h"
#include "net_client.h"

namespace net {

CNetSocket::CNetSocket()
{
	mSockfd = -1;
	memset(&mAddress, 0, sizeof(mAddress));
}

CNetSocket::~CNetSocket()
{
	if(mSockfd > 0)
	{
		close(mSockfd);
	}
}

int CNetSocket::Listen(const string& endpoint, int backlog)
{

	int ret = ParseEndpoint(endpoint);
	if(0 != ret)
	{
		return ret;
	}

	ret = CreateSocket();
	if(0 > ret)
	{
		return ret;
	}

	ret = listen(mSockfd, backlog);
	if(ret < 0)
	{
		return ret;
	}

	return mSockfd;
}

int CNetSocket::ParseEndpoint(const string& endpoint)
{
	size_t colon_pos = endpoint.rfind(':');
    if (colon_pos == string::npos)
    {
        return -1;
    }

	mAddress.sin_family = AF_INET;
    mAddress.sin_addr.s_addr = inet_addr(endpoint.substr(0, colon_pos).c_str());
    mAddress.sin_port = htons((uint16_t)atoi(endpoint.substr(colon_pos+1, endpoint.size()).c_str()));

	return 0;
}

int CNetSocket::CreateSocket()
{
	mSockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(mSockfd < 0)
	{
		return -1;
	}
	
	int ret = SetReuseAddr();
	if(ret < 0)
	{
		close(mSockfd);
		return ret;
	}

	ret = Bind();
	if(ret < 0)
	{
		close(mSockfd);
		return ret;
	}
	
	return mSockfd;
}

int CNetSocket::SetReuseAddr()
{
	int opt = 1;
	if(setsockopt(mSockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		return -2;
	}

	return 0;
}

int CNetSocket::Bind()
{
	if(bind(mSockfd, (struct sockaddr*)&mAddress, sizeof(mAddress)) < 0)
	{
		return -1;
	}

	return 0;
}

int CNetSocket::Process()
{
	while(mSockfd > 0)
	{
		if(0 < mt_wait_events(mSockfd, EPOLLIN, -1))
		{
			socklen_t clientaddr_len = 0;
			struct sockaddr_in remote_addr;
			memset((void*)&remote_addr, 0, sizeof(remote_addr));
			int clientfd = mt_accept(mSockfd, (struct sockaddr*)&remote_addr, &clientaddr_len, 500);
			if(0 > clientfd)
			{
				continue;	
			}

			CNetClient *pNetClient = new CNetClient();
			pNetClient->SetClientfd(clientfd);
			pNetClient->SetLocalAddr(mAddress);
			pNetClient->SetFromAddr(remote_addr);

			if(mt_start_thread((void*)ThreadEntryFunc, (void*)pNetClient) == NULL)
			{
				close(clientfd);
				delete pNetClient;
			}
		}
	}
	return 0;
}

}