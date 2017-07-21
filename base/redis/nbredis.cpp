#include "nbredis.h"


void get_callback(redisAsyncContext *c, void *r, void *privdata) 
{
    rconn *conn = (rconn *)c->data;
    conn->on_execcmd(c, r, privdata);
}

void connect_callback(const redisAsyncContext *c, int status) 
{
    rconn *conn = (rconn *)c->data;
    conn->on_connect((redisAsyncContext *)c, status);
}

void disconnect_callback(const redisAsyncContext *c, int status) 
{
    rconn *conn = (rconn *)c->data;
    conn->on_disconnect((redisAsyncContext *)c, status);
}


rconn::rconn()
{
    memset(m_ip, 0, sizeof(m_ip));
    m_port = 0;
    memset(m_user, 0, sizeof(m_user));

    m_hand = NULL;
    m_loop = 0;
    m_ac = NULL;
}

rconn::~rconn()
{

}


int rconn::init(char *ip, int port, char *user, char *passwd, rhandle *hand, struct ev_loop *loop)
{
    strncpy(m_ip, ip, sizeof(m_ip));
    m_port = port;
    strncpy(m_user, user, sizeof(m_user));
    strncpy(m_passwd, passwd, sizeof(m_passwd));
    m_hand = hand;
    m_loop = loop;

    m_ac = redisAsyncConnect(m_ip, m_port);
    if (m_ac->err)
    {
        return -1;
    }

    m_ac->data = this;

    redisLibevAttach(m_loop, m_ac); 
    redisAsyncSetConnectCallback(m_ac, connect_callback);
    redisAsyncSetDisconnectCallback(m_ac, disconnect_callback);

    return 0;
}

int rconn::disconnect()
{
    redisAsyncDisconnect(m_ac);
    return 0;
}


int rconn::execcmd(char *cmd, int cmdlen, void *userdata)
{
    redisAsyncCommand(m_ac, get_callback, userdata, cmd);

    return 0;
}


int rconn::on_execcmd(redisAsyncContext *c, void *r, void *privdata)
{
    m_hand->on_execcmd(c, r, privdata);
    return 0;
}

int rconn::on_connect(redisAsyncContext *c, int status)
{
    m_hand->on_connect(c, status);
    return 0;
}

int rconn::on_disconnect(redisAsyncContext *c, int status)
{
    m_hand->on_disconnect(c, status);
    return 0;
}
