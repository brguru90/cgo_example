#include "api_req.h"
#include <iterator>
#include <functional>
#include <sstream>
#include <fstream>
#include <map>


class api_req_async
{
private:
    void *data;
    uv_loop_t *loop = nullptr;
    CURLM *curl_handle = nullptr;
    uv_timer_t timeout;
    void add_request_to_event_loop(request_input *req_input, response_data *response_ref, int debug);
    void on_request_complete();
    curl_context_t *create_curl_context(curl_socket_t sockfd);
    int start_timeout(CURLM *multi, long timeout_ms, void *userp);
    int handle_socket(CURL *easy, curl_socket_t s, int action, void *userp, void *socketp);

public:
    int thread_id = -1;
    long loop_addrs_int;
    api_req_async(int th_id);
    ~api_req_async();
    void *run(void *data);
    void *get_result();
    void (api_req_async::*on_timeout_ptr)(uv_timer_t *req);
};

typedef struct ThreadData
{
    request_input *req_inputs_ptr;
    response_data *response_ref_ptr;
    int thread_id;
    int debug_flag;
    thread_pool_data th_pool_data;
    api_req_async *api_req_async_on_thread;
    BytesType raw_bytes;
} thread_data;

typedef void (*ipc_received_cb_data_type)(StringType *raw_response, uv_stream_t *client_stream);


class my_tcp_server
{
private:
    uv_loop_t *loop;
    uv_tcp_t server;
    int DEFAULT_PORT;
    int connections_i=0;
    typedef struct
    {
        uv_write_t req;
        uv_buf_t buf;
    } write_req_t;


    // typedef std::map<int, StringType> responses_type;
    // responses_type responses;

    // typedef struct loop_data_struct
    // {
    //    ipc_received_cb_data_type *get_received_data_cb;
    //    responses_type *responses_ref;
    // } loop_data_type;
    // loop_data_type loop_data;


    void on_new_connection(uv_stream_t *server, int status);
    StringType echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);

public:
    int server_ready = 0;
    struct sockaddr_in addr;
    my_tcp_server(int port);
    void register_ipc_received_callback(ipc_received_cb_data_type *get_received_data_cb);
    void write2client(uv_stream_t *stream, char *data, size_t len2);
    int start_server();
    void stop_server();
};

class my_tcp_client
{
private:
    uv_loop_t *loop;
    uv_tcp_t client;
    int DEFAULT_PORT;
    ipc_received_cb_data_type get_received_data_cb;

    typedef struct 
    {
        uv_write_t req;
        uv_buf_t buf;
    } write_req_t;


    void on_connect(uv_connect_t *client, int status);
    void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf, ipc_received_cb_data_type *cb);

public:
    struct sockaddr_in addr;
    my_tcp_client(int port);
    void register_ipc_received_callback(ipc_received_cb_data_type *get_received_data_cb);
    uv_write_t *write2server(uv_stream_t *stream, char *data, size_t len2, uv_write_t *req);
    uv_write_t *stream2server(uv_stream_t *stream, StringType data,int chunk_size);
    void read_response(uv_stream_t *stream, ipc_received_cb_data_type *cb);
    void free_write_req(uv_write_t *req);
    int start_client();
    void stop_client();
};



void my_strcpy(StringType &dest, char *src, int length);
StringType my_str_slice(StringType src, int start, int length);
int isSubString(StringType &dest, char end_of_data[]);
long long get_current_time();
