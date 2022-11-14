#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


#include <pthread.h>
#include <time.h>

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdbool.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif


#include "curl/curl.h"


typedef struct SockType
{
    char ip[20];
    int socket;
    SSL *secureConn;
    int port;
} sock_type;

typedef struct ResponseData
{
    char *response;
    time_t before_connect_time;
    time_t connect_time;
    time_t time_at_first_byte;
} response_data;

void run_bulk_api_request(char *s);
void *goCallback_wrap(void *vargp);
void goCallback(int myid);

response_data send_raw_request(char *host, in_port_t port, bool secure, char *raw_req, int debug);