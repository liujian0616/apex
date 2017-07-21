/**
 * @brief service impl
 */

#ifndef __user_HEAD_H__
#define __user_HEAD_H__

#include <string>
#include "user.pb.h"

using namespace user;
/**
 * @brief 默认生成的服务实现类型
 */
class CRpcUserServiceImpl : public UserService
{
public:

    // 构造函数及析构函数
    CRpcUserServiceImpl() {}
    virtual ~CRpcUserServiceImpl() {}

    
    // RPC方法函数定义
    virtual void AddUser(::google::protobuf::RpcController* controller,\
                const UserInfo* request, \
                UserID* response, \
                ::google::protobuf::Closure* done);

    // RPC方法函数定义
    virtual void GetUser(::google::protobuf::RpcController* controller,\
                const UserID* request, \
                UserInfo* response, \
                ::google::protobuf::Closure* done);


};

#endif


