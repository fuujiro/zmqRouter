#include <zmq.h>
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
    void* worker = zmq_socket(context, ZMQ_REP);
    zmq_setsockopt (worker, ZMQ_IDENTITY, argv[3], sizeof argv[3]);
    int rec = zmq_connect(worker, addr);
    if (rec) {
        printf("zmq_connect %s error:%d\n", addr, rec);
        zmq_close(worker);
        zmq_ctx_term(context);
        return -2;
    }

    printf("worker zmq_connect %s done!\n", addr);

    char buf[128] = {0};

    while (1) {
        rec = zmq_recv(worker, buf, sizeof(buf), 0);
        printf("[%ld] recv request(%s) from client, rec = %d, request.len = %d\n", time(NULL), buf, rec, strlen(buf));
        snprintf(buf, sizeof(buf), "worker=%s&result=world&time=%ld", argv[3], time(NULL));
        rec = zmq_send(worker, buf, strlen(buf), 0);
        printf("[%ld] send reply(%s) to client, rec = %d, reply.len = %d\n", time(NULL), buf, rec, strlen(buf));
        printf("[%ld]------------------------\n\n", time(NULL));
    }

    zmq_close(worker);
    zmq_ctx_term(context);

    printf("good bye and good luck!\n");
    return 0;
}