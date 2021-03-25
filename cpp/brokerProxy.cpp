#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
 
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
 
  zmq_proxy(router, dealer, NULL);
 
  zmq_close(router);
  zmq_close(dealer);
  zmq_ctx_term(context);
  printf("[%ld] good bye and good luck!\n", time(NULL));
 
  return 0;
}