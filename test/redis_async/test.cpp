#include <stdio.h>
#include "nbredis.h"
#include "ev.h"

class nbrhandle : public rhandle
{

    int on_execcmd(redisAsyncContext *c, void *r, void *privdata)
    {
        printf("%s\n", __func__);

        redisReply *reply = (redisReply *)r;
        if (reply == NULL)
        {
            printf("reply is null\n");
            return -1;
        }

        printf("privdata[%s]: %s\n", (char*)privdata, reply->str);

        /* Disconnect after receiving the reply to GET */
        //redisAsyncDisconnect(c);

        return 0;
    }

    int on_connect(redisAsyncContext *c, int status)
    {
        printf("%s\n", __func__);
        if (status != REDIS_OK) {
            printf("Error: %s\n", c->errstr);
            return -1;
        }   
        printf("Connected...\n");

        return 0;
    }

    int on_disconnect(redisAsyncContext *c, int status)
    {
        printf("%s\n", __func__);
        if (status != REDIS_OK) {
            printf("Error: %s\n", c->errstr);
            return -1;
        }   
        printf("Disconnected...\n");

        return 0;
    }
};

int main()
{
    char redis_ip[16] = "192.168.1.118";
    int redis_port = 20000;
    char user[256] = {0};
    char passwd[256] = {0};

    nbrhandle rhand;
    rconn conn;
    printf("conn:%p\n", &conn);
    struct ev_loop *loop = EV_DEFAULT;
    conn.init(redis_ip, redis_port, user, passwd, &rhand, loop);

    char cmd[] = "get key";
    char str[] = "hello";
    conn.execcmd(cmd, strlen(cmd), str);

    char cmd2[] = "set key abc";
    conn.execcmd(cmd2, strlen(cmd2), str);

    conn.disconnect();

    while(1)
    {
        ev_loop(loop, 0);
    }

    return 0;
}
