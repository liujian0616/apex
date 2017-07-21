#ifndef _NET_CLIENT_H_
#define _NET_CLIENT_H_

#include <iostream>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "mt_incl.h"

namespace net {
void ThreadEntryFunc(void* arg);

class CNetClient
{
public:
    /**
     * 构造析构函数
     */
    CNetClient();

    ~CNetClient();

    /**
     * 客户端sockfd
     *
     */
    void SetClientfd(int sockfd){
        _clientfd = sockfd;
    };
    int GetClientfd(void){
        return _clientfd;
    };

    /**
     * 报文目的地址设置与读取接口
     *
     * @param addr 输入地址信息
     */ 
    void SetLocalAddr(const struct sockaddr_in& local_addr) {
        memcpy(&_local_addr, &local_addr, sizeof(_local_addr));
    };
    void GetLocalAddr(struct sockaddr_in& local_addr) {
        memcpy(&local_addr, &_local_addr, sizeof(_local_addr));
    };

    /**
     * 报文来源地址设置与读取接口
     *
     * @param addr 输入地址信息
     */ 
    void SetFromAddr(const struct sockaddr_in& from_addr) {
        memcpy(&_from_addr, &from_addr, sizeof(_from_addr));
    };
    void GetFromAddr(struct sockaddr_in& from_addr) {
        memcpy(&from_addr, &_from_addr, sizeof(_from_addr));
    };

    /**
     * 报文来源时间戳信息
     *
     * @param time_rcv 输入输出时间戳
     */ 
    void SetRcvTimestamp(const struct timeval& time_rcv) {
        memcpy(&_time_rcv, &time_rcv, sizeof(time_rcv));
    };
    void GetRcvTimestamp(struct timeval& time_rcv) {
        memcpy(&time_rcv, &_time_rcv, sizeof(time_rcv));
    };

    /**
     * 接收报文内容
     *
     * @param timeout 超时时间
     */ 
    int RecvFromClient(int timeout);

    /**
     * 报文内容信息存储, 可选调用
     *
     * @param pkg 报文起始指针
     * @param len 报文长度
     */ 
    void SetReqPkg(const void* pkg, int pkg_len) {
        _msg_buff.assign((char*)pkg, pkg_len);
    };
    const string& GetReqPkg() {
        return _msg_buff;
    };

    /**
     * 给客户端回包
     *
     * @param pkg 报文起始指针
     * @param len 报文长度
     */
    int SendToClient(void *pkg, int32_t msg_len, int timeout) {
        return mt_send(_clientfd, pkg, msg_len, 0, 500);
    };

protected:
    int _clientfd;                      //客户端sockfd
    struct sockaddr_in _from_addr;      // 报文来源IP
    struct sockaddr_in _local_addr;     // 报文接收的本地IP
    struct timeval _time_rcv;           // 收包的时间戳
    string _msg_buff;                   // 报文拷贝一份, 多线程处理需要

};

#define SRPC_PACK_PKG(pkg, len, head, body) ({ \
        int32_t ret; \
        if ((NULL == body) || (body->ByteSize() == 0)) { \
            ret = SrpcPackPkgNoBody(pkg, len, head); \
        } else { \
            ret = SrpcPackPkg(pkg, len, head, body); \
        } \
        ret; \
    })

}


#endif //_NET_CLIENT_H_