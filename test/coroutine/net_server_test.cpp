#include <iostream>
#include "net_socket.h"
#include "srpc_service.h"
#include "user_service.h"
#include "mt_api.h"

using namespace std;
using namespace net;
using namespace srpc;

int main(int argc, char const *argv[])
{
	if(!mt_init_frame())
	{
		cout << "mt_init_frame failed" << endl;
		return -1;
	}

	CMethodManager* pCMethodManager = CMethodManager::Instance();
	if(NULL == pCMethodManager)
	{
		cout << "pCMethodManager is NULL" << endl;
		return -1;
	}

	CRpcUserServiceImpl *service = new CRpcUserServiceImpl();
	pCMethodManager->RegisterService(service, NULL);

	CNetSocket *pCNetSocket = new CNetSocket();
	int ret = pCNetSocket->Listen("10.0.201.165:9988", 1024);

	IMtTaskList task_list;
	task_list.push_back(pCNetSocket);

	cout << "IMtTaskList.Size:" << task_list.size() << endl;

	mt_exec_all_task(task_list);

	/*
	cout << "ret:" << ret << " start" << endl;
	for (unsigned int i = 0; i < task_list.size(); i++)
	{
		IMtTask *task = task_list[i];
		int result = task->GetResult();

		if (result < 0)
		{
			cout << "task(" << i <<") failed, result:" << result << endl;
		}
	}
	*/
	return 0;
}