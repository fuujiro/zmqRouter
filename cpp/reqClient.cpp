#include <zmq.h>
#include <zmq.hpp>
#include "zhelpers.hpp"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
 
int main(int argc, char** argv)
{
    if (argc < 4) {
        printf("usage : %s ip port index\n", argv[0]);
        return -1;
    }

    int major, minor, patch;
    zmq_version (&major, &minor, &patch);
    printf ("Current 0MQ version is %d.%d.%d\n", major, minor, patch);
    printf("===========================================\n\n");

    char addr[128] = {0};
    snprintf(addr, sizeof(addr), "tcp://%s:%s", argv[1], argv[2]);

    void* context = zmq_ctx_new();
    void* request = zmq_socket(context, ZMQ_REQ);

    // std::string identity = s_set_id(worker);
    // printf("%s\n",ZMQ_IDENTITY);

    int rec = zmq_connect(request, addr);
    if (rec) {
        printf("zmq_connect %s error:%d\n", addr, rec);
        zmq_close(request);
        zmq_ctx_term(context);
        return -2;
    }

    printf("client zmq_connect %s done!\n", addr);

    char buf[128] = {0};

    while (1) {
        snprintf(buf, sizeof(buf), "index=%s&cmd=hello&time=%ld", argv[3], time(NULL));
        rec = zmq_send(request, buf, strlen(buf), 0);
        printf("[%ld] send request(%s) to server, rec = %d, request.len = %d\n", time(NULL), buf, rec, strlen(buf));
        rec = zmq_recv(request, buf, sizeof(buf), 0);
        printf("[%ld] recv reply(%s) from server, rec = %d, reply.len = %d\n", time(NULL), buf, rec, strlen(buf));
        printf("[%ld] --------------------------\n\n", time(NULL));
        sleep(3);
    }

    zmq_close(request);
    zmq_ctx_term(context);

    printf("good bye and good luck!\n");
    return 0;
}