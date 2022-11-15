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


#include <curl/curl.h>




typedef struct ResponseData
{
    char *response;
    time_t before_connect_time;
    long connect_time_microsec;
    long time_at_first_byte_microsec;
} response_data;

void run_bulk_api_request(char *s);
void *goCallback_wrap(void *vargp);
extern void goCallback(int myid);

response_data send_raw_request(char *url, bool secure, char *raw_req, int debug);