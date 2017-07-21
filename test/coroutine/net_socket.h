#ifndef _NET_SOCKET_H_
#define _NET_SOCKET_H_

#include <iostream>
#include "mt_incl.h"

namespace net {

class CNetSocket : public IMtTask
{
public:
	CNetSocket();
	~CNetSocket();
	
	virtual int Process();

	//创建监听套接字
	int Listen(const string& endpoint, int backlog);

private:
	//解析endpoint地址信息
	int ParseEndpoint(const string& endpoint);

	//创建套接字
	int CreateSocket();

	//设置端口可重用
	int SetReuseAddr();

	//绑定端口
	int Bind();

private:
	int mSockfd; //监听sockfd
	struct sockaddr_in mAddress; //服务端地址
};



}

#endif //_NET_SOCKET_H_