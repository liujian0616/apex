#include <sys/epoll.h>
#include "net_client.h"
#include "srpc_proto.h"
#include "srpc_service.h"
#include "srpc_comm.h"

using namespace srpc;

namespace net {

CNetClient::CNetClient() 
{
    _clientfd = -1;
    memset(&_from_addr, 0, sizeof(_from_addr));
    memset(&_local_addr, 0, sizeof(_local_addr));
    memset(&_time_rcv, 0, sizeof(_time_rcv));
}

CNetClient::~CNetClient()
{
    if(_clientfd != -1)
    {
        close(_clientfd);
        _clientfd = -1;
    }
}

int CNetClient::RecvFromClient(int timeout) 
{
    char *buf = NULL;
    int length;
    int ret = mt_tcp_check_recv_v3(_clientfd, &buf, length, 0, timeout, SrpcCheckPkgHeaderLen);
    if(0 == ret)
    {
        this->SetReqPkg(buf, length);
    }

    if(NULL != buf)
    {
    	free(buf);
    }
    return ret;
}

void ThreadEntryFunc(void* arg)
{
    CNetClient *pMicroClient = (CNetClient*)arg;
    if(NULL == pMicroClient)
    {
        return; 
    }
    
    for(;;)
    {
        cout << "start time:" << mt_time_ms() << endl;
        int events = mt_wait_events(pMicroClient->GetClientfd(), EPOLLIN, -1);
        if(events & EPOLLIN)
        {
            Message* request  = NULL;
            Message* response = NULL;
            Service* service = NULL;
            CMethodInfo* methodInfo = NULL;
            const MethodDescriptor* m_method = NULL;

            CRpcHead rpc_head;
            CRpcCtrl rpcCtrl;

            //1、读取数据
            int ret = pMicroClient->RecvFromClient(500);
            if(ret == -7)
            {
            	cout << pMicroClient->GetClientfd() << " clientfd exit" << endl;
            	break;
            }

            if(0 > ret)
            {
                cout << "RecvFromClient failed, ret:" << ret << endl;
          		break;
            }

            cout << "Size:" << pMicroClient->GetReqPkg().size() << endl;

            //2、解包头
            ret = SrpcUnpackPkgHead(pMicroClient->GetReqPkg().data(), pMicroClient->GetReqPkg().size(), &rpc_head);
            if(ret != SRPC_SUCCESS)
            {
                cout << "SrpcUnpackPkgHead failed" << endl;
                rpc_head.set_err(ret);
                goto EXIT_ERR;
            }

            //3、获取RPC调用的方法
            methodInfo = CMethodManager::Instance()->GetMethodInfo(rpc_head.method_name());
            if(NULL == methodInfo)
            {
                cout << "GetMethodInfo is NULL" << endl;
                rpc_head.set_err(SRPC_ERR_INVALID_METHOD_NAME);
                goto EXIT_ERR;
            }
            
            request  = methodInfo->m_request->New();
            response = methodInfo->m_response->New();
            service = methodInfo->m_service;
            m_method = methodInfo->m_method;

            //4、解包体
            ret = SrpcUnpackPkg(pMicroClient->GetReqPkg().data(), pMicroClient->GetReqPkg().size(), &rpc_head, request);
            if (ret != SRPC_SUCCESS)
            {
                rpc_head.set_err(ret);
                goto EXIT_ERR;
            }
            
            if (!rpc_head.flow_id())
            {
                rpc_head.set_flow_id((uint64_t)random());
            }

            //5、RPC调用
            cout << "process request:\n" << request->DebugString() << endl;
            service->CallMethod(m_method, &rpcCtrl, request, response, NULL);
            if(rpcCtrl.Failed())
            {
                rpc_head.set_err(SRPC_ERR_SERVICE_IMPL_FAILED);
                goto EXIT_ERR;
            }

EXIT_ERR:
            //6、打包
            int32_t msg_len;
            char* rsp_buff = NULL;
            ret = SRPC_PACK_PKG(&rsp_buff, &msg_len, &rpc_head, response);
            if (ret < 0) {
                cout << "SRPC_PACK_PKG failed" << endl;
                if(NULL != rsp_buff)
	            {
	            	free(rsp_buff);
	            }
                continue;
            }

            pMicroClient->SendToClient((void*)rsp_buff, msg_len, 500);
            if(NULL != rsp_buff)
            {
            	free(rsp_buff);
            }
            cout << "end time:" << mt_time_ms() << endl;
        }
    }

    delete pMicroClient;
}

}