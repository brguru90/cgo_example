#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <pthread.h>
#include <time.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <inttypes.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

#include <curl/curl.h>
#include <uv.h>

typedef struct curl_context_s
{
    uv_poll_t poll_handle;
    curl_socket_t sockfd;
} curl_context_t;

typedef struct CurlHandlers
{
    uv_loop_t *loop;
    CURLM *curl_handle;
    uv_timer_t timeout;
} curl_handlers_t;

typedef struct Headers
{
    char *header;
} headers_type;

typedef struct AdditionalDetails
{
    char *uuid;
    int total_requests;
    int total_threads;
} additional_details;

typedef struct SingleRequestInput
{
    char *uid;
    char *url;
    char *method;
    headers_type *headers;
    char *cookies;
    int headers_len;
    char *body;
    int time_out_in_sec;
} request_input;

struct memory
{
    char *data;
    size_t size;
};

typedef struct ResponseData
{
    int Debug;
    char *Uid;
    char *Response_header;
    char *Response_body;
    struct memory Resp_header;
    struct memory Resp_body;
    // time_t before_connect_time; //long int
    long long Before_connect_time_microsec;
    long long After_response_time_microsec;
    long long Connected_at_microsec;
    long long First_byte_at_microsec;
    long long Finish_at_microsec;
    long Connect_time_microsec;
    long Time_to_first_byte_microsec;
    long Total_time_from_curl_microsec;
    long Total_time_microsec;
    int Status_code;
    int Err_code;
} response_data;

typedef struct ThreadPoolData
{
    int start_index;
    int end_index;
    int pid;
    char *uuid;
    bool full_index;
} thread_pool_data;

typedef struct BytesType
{
    unsigned char *ch;
    long length;
} bytes_type;

typedef struct StringType
{
    char *ch;
    long length;
} string_type;

typedef struct ResponseDeserialized
{
    response_data *data;
    int len;
    int start;
    int end;
} response_deserialized_type;

// void * ptr_at(void **ptr, int idx) {
//     return ptr[idx];
// }

#ifdef __cplusplus
extern "C"
{
#endif

    response_deserialized_type *json_to_thread_data(char *response_json);
    char *thread_data_to_json(response_data response_ref_ptr, int length, int start, int end);
    void send_request_in_concurrently(request_input *req_inputs, response_data *response_ref, int total_requests, int total_threads, int debug);

#ifdef __cplusplus
}
#endif