#include "api_req.h"
#include <iterator>
#include <functional>
#include <sstream>

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
    void* get_result();
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

