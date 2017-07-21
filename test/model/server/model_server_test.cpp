#include <iostream>
#include "model_pbrpc.h"
#include "model_server.h"
#include "user_service.h"
#include "mt_api.h"
#include "model_mgr.h"

using namespace std;
using namespace model;

int main(int argc, char const *argv[])
{
	if(!mt_init_frame())
	{
		cout << "mt_init_frame failed" << endl;
		return -1;
	}

	CModelMgr::Instance()->RegisterService(new CRpcUserServiceImpl());

	CTModelServer<CModelRpc> *pCTModelServer1 = new CTModelServer<CModelRpc>();
	CTModelServer<CModelRpc> *pCTModelServer2 = new CTModelServer<CModelRpc>();
	int ret = pCTModelServer1->Listen("10.0.201.165:9988", 1024);
	if(0 > ret)
	{
		return -1;
	}
	
	CModelMgr::Instance()->AddServerItem(ret, pCTModelServer1);
	ret = pCTModelServer2->Listen("10.0.201.165:9989", 1024);
	if(0 > ret)
	{
		return -1;
	}
	CModelMgr::Instance()->AddServerItem(ret, pCTModelServer2);

	CModelMgr::Instance()->Serve();

	return 0;
}