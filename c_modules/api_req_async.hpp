#include "api_req.h"

class api_req_async
{
private:
    uv_loop_t *loop;
    CURLM *curl_handle;
    uv_timer_t timeout;
    void add_request_to_event_loop(request_input *req_input, response_data *response_ref, int debug);
    void on_request_complete(CURLMcode res);
    curl_context_t *create_curl_context(curl_socket_t sockfd);
    void curl_close_cb(uv_handle_t *handle);
    void destroy_curl_context(curl_context_t *context);
    void curl_perform(uv_poll_t *req, int status, int events);
    void on_timeout(uv_timer_t *req);
    int start_timeout(CURLM *multi, long timeout_ms, void *userp);
    int handle_socket(CURL *easy, curl_socket_t s, int action, void *userp, void *socketp);

public:
    api_req_async(/* args */);
    ~api_req_async();
    void* run(void *data);
};


typedef struct ThreadData
{
    request_input *req_inputs_ptr;
    response_data *response_ref_ptr;
    int thread_id;
    int debug_flag;
    thread_pool_data th_pool_data;
    api_req_async api_req_async_on_thread;
} thread_data;