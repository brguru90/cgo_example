#include "api_req.h"

class api_req_async
{
private:
    static void add_request_to_event_loop(request_input *req_input, response_data *response_ref, int debug);
    static void on_request_complete(CURLMcode res);
    static void curl_close_cb(uv_handle_t *handle);
    static void destroy_curl_context(curl_context_t *context);
    static void curl_perform(uv_poll_t *req, int status, int events);
    static void on_timeout(uv_timer_t *req);
    static int start_timeout(CURLM *multi, long timeout_ms, void *userp);
    static int handle_socket(CURL *easy, curl_socket_t s, int action, void *userp, void *socketp);
    static curl_context_t *create_curl_context(curl_socket_t sockfd);
    static size_t response_writer(void *data, size_t size, size_t nmemb, void *userp);


public:
    api_req_async(/* args */);
    ~api_req_async();
    void* run(void *data);
};

class HttpRequestMgr{
public:
    HttpRequestMgr():curl_handlers((curl_handlers_t){.loop=NULL,.curl_handle=nullptr}){};

    static HttpRequestMgr& instance(){
        static HttpRequestMgr s_instance;
        return s_instance;
    }
    bool init();
    curl_handlers_t getCurlHandler(){ return curl_handlers;}
    
private:
    curl_handlers_t curl_handlers;
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