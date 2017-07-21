//g++ -o client_raw client_raw.cpp -I ../../inc -L ../../lib -lapexbase -lev

#include <stdio.h>
#include <string.h>
#include "util_net.h"

using namespace net;

class client_raw : public tcp_client_handler_raw
{
    public:
        client_raw(const net::conf_section& data) : tcp_client_handler_raw(data) { }
        ~client_raw() { }

        virtual void on_connect(tcp_client_channel* channel)
        {
            printf("%s\n", __func__);
            channel->try_send((uint8_t*)"hello,world",strlen("hello,world"));
        }
        virtual void on_closing(tcp_client_channel* channel)
        {
            printf("%s\n", __func__);
        }
        virtual void on_error(ERRCODE err, tcp_client_channel* channel)
        {
            printf("%s\n", __func__);
        }


        virtual void on_timer(uint8_t timer_id, tcp_client_channel* channel)
        {
            printf("%s\n", __func__);
        }

        //return 0 包不完整 >0 完整的包 
        virtual int32_t on_recv(const uint8_t *pkg, const uint32_t len, const tcp_client_channel* channel)
        {
            printf("%s\n", __func__);
        }
        virtual void on_pkg(const uint8_t *pkg, const uint32_t len, const tcp_client_channel* channel)
        {
            printf("%s\n", __func__);
        }

};


int main(int argc, char** argv)
{
    conf_section conf("conf.ini", "olinestatus_clt");
    client_raw *client_raw1 = new client_raw(conf);
    struct ev_loop *loop = EV_DEFAULT;
    add_tcp_client(loop, client_raw1);

    run(loop);

    printf("hello,world\n");

    return 0;
}
