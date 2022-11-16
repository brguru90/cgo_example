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

typedef struct Headers
{
    char *header;
} headers_type;

typedef struct SingleRequestInput
{
    char *url;
    char *method;
    headers_type *headers;
    int headers_len;
    char *body;
} request_input;

typedef struct ResponseData
{
    char *response_header;
    char *response_body;
    // time_t before_connect_time; //long int
    long long before_connect_time;
    long connect_time_microsec;
    long time_at_first_byte_microsec;
    long total_time_microsec;
    int status_code;
} response_data;

typedef struct RequestResponse
{
    request_input *req_inputs_ptr;
    response_data *response_ref_ptr;
    int *debug_flag;
} request_response;

typedef struct ThreadData
{
    request_response req_res;
    int thread_id;
} thread_data;

struct memory
{
    char *data;
    size_t size;
};

void run_bulk_api_request(char *s);
void *goCallback_wrap(void *vargp);
extern void goCallback(int myid);

void send_request_concurrently(request_input *req_inputs, response_data *response_ref,int total_requests, int debug);
void send_raw_request(request_input *req_input, response_data *response_ref, int debug);