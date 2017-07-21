/**
 * @brief service impl
 */
#include <iostream>
#include <string>
#include "user.pb.h"
#include "user_service.h"

#include "srpc_channel.h"
#include "srpc_ctrl.h"
#include "mt_api.h"
#include "srpc.pb.h"

using namespace user;
using namespace std;
using namespace srpc;
using namespace NS_MICRO_THREAD;


/**
 * @brief RPC方法函数定义
 */
void CRpcUserServiceImpl::AddUser(::google::protobuf::RpcController* controller,
            const UserInfo* request,
            UserID* response,
            ::google::protobuf::Closure* done)
{
	cout << request->name() << " " << request->age() << " " << request->address() << endl;
	response->set_user_id(request->age());
}

/**
 * @brief RPC方法函数定义
 */
void CRpcUserServiceImpl::GetUser(::google::protobuf::RpcController* controller,
            const UserID* request,
            UserInfo* response,
            ::google::protobuf::Closure* done)
{
	cout << "userId:" << request->user_id() << endl;
	response->set_name("cat");
    response->set_age(25);
    response->set_address("广东省");
}


