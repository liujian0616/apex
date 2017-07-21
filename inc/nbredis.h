#include <stdint.h>
#include <stdio.h>
#include "hiredis.h"
#include "async.h"
#include "adapters/libev.h"
#include "ev.h"

class rhandle
{
    public:
        virtual int on_execcmd(redisAsyncContext *c, void *r, void *privdata) = 0;
        virtual int on_connect(redisAsyncContext *c, int status) = 0;
        virtual int on_disconnect(redisAsyncContext *c, int status) = 0;
};


class rconn
{

    public:
        rconn();
        ~rconn();


        int init(char *ip, int port, char *user, char *passwd, rhandle *hand, struct ev_loop *loop);
        int disconnect();

        int execcmd(char *cmd, int cmdlen, void *userdata);

        int on_execcmd(redisAsyncContext *c, void *r, void *privdata);
        int on_connect(redisAsyncContext *c, int status);
        int on_disconnect(redisAsyncContext *c, int status);

    private:
        redisAsyncContext *m_ac;
        struct ev_loop  *m_loop;

        char m_ip[32];
        int  m_port;
        char m_user[256];
        char m_passwd[256];

        rhandle *m_hand;
};
