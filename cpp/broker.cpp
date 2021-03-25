#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
 
/*
* 将客户端请求公平的转发到Dealer。
* 然后由Dealer内部负载均衡的派发任务到各个Worker。
*/
int DoRouter(void* router, void* dealer)
{
    while (1) {
        int64_t more = 1;
        zmq_msg_t message;
        zmq_msg_init(&message);
        int rc = zmq_msg_recv(&message, router, 0);
        size_t more_size = sizeof(more);
        zmq_getsockopt(router, ZMQ_RCVMORE, &more, &more_size);
        zmq_msg_send(&message, dealer, more ? ZMQ_SNDMORE : 0);
        printf("[%ld] router deliver request to dealer. rc = %d, more = %ld\n", time(NULL), rc, more);
        zmq_msg_close(&message);
        if (!more) {
            break; // Last message part
        }
    }

    printf("[%ld]----------DoRouter----------\n\n", time(NULL));
    return 0;
}
 
/*
* Dealer将后端Worker的应答数据转发到Router。
* 然后由Router寻址将应答数据准确的传递给对应的client。
* 值得注意的是，Router对client的寻址方式，得看client的‘身份’。
* 临时身份的client，Router会为其生成一个uuid进行标识。
* 永久身份的client，Router直接使用该client的身份。
*/
int DoDealer(void* router, void* dealer)
{
    while (1) {
        int64_t more = 1;
        zmq_msg_t message;
        // Process all parts of the message
        zmq_msg_init(&message);
        int rc = zmq_msg_recv(&message, dealer, 0);
        size_t more_size = sizeof (more);
        zmq_getsockopt(dealer, ZMQ_RCVMORE, &more, &more_size);
        zmq_msg_send(&message, router, more ? ZMQ_SNDMORE : 0);
        printf("[%ld] dealer deliver reply to router. rc = %d, more = %ld\n", time(NULL), rc, more);
        zmq_msg_close(&message);
        if (!more) {
            break; // Last message part
        }
    }

    printf("[%ld]----------DoDealer----------\n\n", time(NULL));
    return 0;
}
 
int main(int argc, char** argv)
{
    if (argc < 5) {
        printf("usage : %s router_ip router_port dealer_ip dealer_port\n", argv[0]);
        return -1;
    }

    int major, minor, patch;
    zmq_version (&major, &minor, &patch);
    printf ("Current 0MQ version is %d.%d.%d\n", major, minor, patch);
    printf("===========================================\n\n");

    char addr[128] = {0};
    void* context = zmq_ctx_new();

    snprintf(addr, sizeof(addr), "tcp://%s:%s", argv[1], argv[2]);
    void* router = zmq_socket(context, ZMQ_ROUTER);
    int rc = zmq_bind(router, addr);
    printf("[%ld] router bind %s %s.\n", time(NULL), addr, (rc ? "error" : "ok"));
    if (rc) {
        printf("[%ld] router bind error : %s\n", time(NULL), zmq_strerror(zmq_errno()));
        zmq_close(router);
        zmq_ctx_term(context);
        return -2;
    }
 
 
    snprintf(addr, sizeof(addr), "tcp://%s:%s", argv[3], argv[4]);
    void* dealer = zmq_socket(context, ZMQ_DEALER);
    rc = zmq_bind(dealer, addr);
    printf("[%ld] dealer bind %s %s.\n", time(NULL), addr, (rc ? "error" : "ok"));
    if (rc) {
        printf("[%ld] dealer bind error : %s\n", time(NULL), zmq_strerror(zmq_errno()));
        zmq_close(router);
        zmq_close(dealer);
        zmq_ctx_term(context);
        return -3;
    }
 
 
    zmq_pollitem_t items [] = {
                                {router, 0, ZMQ_POLLIN, 0}
                            , {dealer, 0, ZMQ_POLLIN, 0} };
    while (1) {
        rc = zmq_poll(items, sizeof(items)/sizeof(zmq_pollitem_t), 1000);
        if (rc < 0) {
            printf("[%ld] zmq_poll error: %d\n", time(NULL), rc);
            break;
        }

        printf("[%ld] zmq_poll rc = %d\n", time(NULL), rc);
        if (rc < 1) {
            continue;
        }

        if (items[0].revents & ZMQ_POLLIN) {
            /* router可读事件，说明有client请求过来了。 */
            printf("[%ld] zmq_poll catch one router event!\n", time(NULL));
            DoRouter(router, dealer);
        }
        else if (items[1].revents & ZMQ_POLLIN) {
            /* dealer可读事件，说明有worker的应答数据到来了。 */
            printf("[%ld] zmq_poll catch one dealer event!\n", time(NULL));
            DoDealer(router, dealer);
        }
        else {
            printf("[%ld] zmq_poll Don't Care this evnet!\n", time(NULL));
        }
    }

    zmq_close(router);
    zmq_close(dealer);
    zmq_ctx_term(context);
    printf("[%ld] good bye and good luck!\n", time(NULL));

    return 0;
}