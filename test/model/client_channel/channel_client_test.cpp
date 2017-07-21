#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include "mt_incl.h"
#include "srpc_ctrl.h"
#include "user.pb.h"
#include "model_channel.h"

using namespace user;
using namespace srpc;
using namespace model;

CModelChannel channel;

class CUserProcess : public IMtTask
{
public:
	CUserProcess(int idx)
	{
		mID = idx;
	}

	virtual ~CUserProcess(){};

	virtual int Process()
	{
		UserInfo request;
		request.set_name("cat");
 		request.set_age(++CUserProcess::X);
 		request.set_address("深圳市");

 		
 		UserService::Stub stub(&channel);                                                                                                        
 		CRpcCtrl ctrl;
 		UserID response;
 		stub.AddUser(&ctrl, &request, &response, NULL);

 		if(!ctrl.Failed())
 		{
 			cout << "mID : " << mID << " Result: " << response.DebugString() << endl;
 		}
 		else
 		{
 			cout << ctrl.ErrorText() << endl;
 		}

 		return 0;
	}
	
private:
	int mID;
	static int X;
};

int CUserProcess::X = 0;

int main(int argc, char const *argv[])
{
	if(!mt_init_frame())
	{
		cout << "init failed" << endl;
		return -1;
	}

	int ret = channel.Connect("10.0.201.165:9988", 500);
	if(0 > ret)
	{
		cout << "Connect ret:" << ret << endl;
		return -1;
	}

	IMtTaskList task_list;

	for (int i = 0; i < 1; ++i)
	{
		task_list.push_back(new CUserProcess(i+1));
	}

	while(true)
	{
		mt_exec_all_task(task_list);

		for (unsigned int i = 0; i < task_list.size(); ++i)
		{
			IMtTask* it = task_list[i];
			int result = it->GetResult();
			if (0 > result)
			{
				cout << "task (" << i << ") failed, result: " << result << endl;
			}
		}

		mt_sleep(2000);
		break;
	}
	return 0;
}