
#include "api_req_async.hpp"

using namespace std;

volatile bool curl_running = false;

long long get_current_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec) * 1e6) + (tv.tv_usec);
}


static size_t response_writer(void *data, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct memory *mem = (struct memory *)userp;

    char *ptr = (char *)realloc(mem->data, mem->size + realsize + 1);
    if (ptr == NULL)
        return 0; /* out of memory! */

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), data, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;

    return realsize;
}

curl_context_t *api_req_async::create_curl_context(curl_socket_t sockfd)
{
    curl_context_t *context;

    context = (curl_context_t *)malloc(sizeof(*context));

    context->sockfd = sockfd;

    uv_poll_init_socket(loop, &context->poll_handle, sockfd);
    context->poll_handle.data = context;

    return context;
}

static void curl_close_cb(uv_handle_t *handle)
{
    curl_context_t *context = (curl_context_t *)handle->data;
    free(context);
}

static void destroy_curl_context(curl_context_t *context)
{
    uv_close((uv_handle_t *)&context->poll_handle, curl_close_cb);
}

void api_req_async::add_request_to_event_loop(request_input *req_input, response_data *response_ref, int debug)
{
    response_ref->Status_code = -2;
    response_ref->Debug = debug;
    if (debug > 0)
    {
        printf("debug_level%d\n", debug);
        printf("%s\n", req_input->url);
    }
    if (debug > 2)
    {
        printf("cookies=>%s\n\n", req_input->cookies);
    }
    if (debug > 2)
    {
        printf("header count=%d\n\n", req_input->headers_len);
        for (int i = 0; i < req_input->headers_len; i++)
        {
            printf("header=>%s\n\n", req_input->headers[i].header);
        }
        printf("body=>%s\n\n", req_input->body);
    }

    CURL *curl;
    curl = curl_easy_init();
    struct curl_slist *header_list = NULL;
    if (req_input->headers_len > 0)
    {
        for (int i = 0; i < req_input->headers_len; i++)
        {
            curl_slist_append(header_list, req_input->headers[i].header);
        }
    }
    curl_easy_setopt(curl, CURLOPT_PRIVATE, response_ref);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, debug > 3 ? 1L : 0);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, req_input->url);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, 0);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_COOKIE, req_input->cookies);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "cgo benchmark tool");
    if (req_input->body != NULL)
    {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req_input->body);
    }
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, req_input->method);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, req_input->time_out_in_sec);

    response_ref->Resp_header = {0};
    response_ref->Resp_body = {0};
    // from response
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, response_writer);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response_ref->Resp_body);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response_ref->Resp_header);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, response_writer);

    curl_multi_add_handle(curl_handle, curl);
    response_ref->Before_connect_time_microsec = get_current_time();
    if (debug > 1)
    {
        printf("request added to event loop: %s\n", req_input->url);
    }
}

static void on_request_complete(CURLM *curl_handle)
{
    char *done_url;
    CURLMsg *message;
    int pending;
    CURL *easy_handle;
    response_data *response_ref;

    while ((message = curl_multi_info_read(curl_handle, &pending)))
    {
        switch (message->msg)
        {
        case CURLMSG_DONE:
        {
            /* Do not use message data after calling curl_multi_remove_handle() and
           curl_easy_cleanup(). As per curl_multi_info_read() docs:
           "WARNING: The data the returned pointer points to will not survive
           calling curl_multi_cleanup, curl_multi_remove_handle or
           curl_easy_cleanup." */
            easy_handle = message->easy_handle;
            curl_easy_getinfo(easy_handle, CURLINFO_EFFECTIVE_URL, &done_url);
            // printf("\n---done_url=%s\n", done_url);
            curl_easy_getinfo(easy_handle, CURLINFO_PRIVATE, &response_ref);
            // if (res != CURLE_OK)
            // {
            //     response_ref->status_code = -1;
            //     response_ref->err_code = res;
            // }
            response_ref->After_response_time_microsec = get_current_time();

            curl_off_t start = -1, connect = -1, total = -1;
            struct memory body = {0}, header = {0};
            // from response
            curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, response_writer);
            curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, (void *)&body);
            curl_easy_setopt(easy_handle, CURLOPT_HEADERDATA, &header);
            curl_easy_setopt(easy_handle, CURLOPT_HEADERFUNCTION, response_writer);

            curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &response_ref->Status_code);
            CURLcode res = curl_easy_getinfo(easy_handle, CURLINFO_CONNECT_TIME_T, &connect);
            if (CURLE_OK != res)
            {
                connect = -1;
            }
            res = curl_easy_getinfo(easy_handle, CURLINFO_STARTTRANSFER_TIME_T, &start);
            if (CURLE_OK != res)
            {
                start = -1;
            }
            res = curl_easy_getinfo(easy_handle, CURLINFO_TOTAL_TIME_T, &total);
            if (CURLE_OK != res)
            {
                total = -1;
            }

            response_ref->Connect_time_microsec = connect;
            response_ref->Time_to_first_byte_microsec = start;
            response_ref->Total_time_from_curl_microsec = total;
            response_ref->Total_time_microsec = (response_ref->After_response_time_microsec - response_ref->Before_connect_time_microsec);
            response_ref->Connected_at_microsec = response_ref->Before_connect_time_microsec + connect;
            response_ref->First_byte_at_microsec = response_ref->Before_connect_time_microsec + start;
            response_ref->Finish_at_microsec = response_ref->Before_connect_time_microsec + response_ref->Total_time_microsec;

            response_ref->Response_header = response_ref->Resp_header.data;
            response_ref->Response_body = response_ref->Resp_body.data;

            if (response_ref->Debug > 2)
            {
                printf("status_code=%d\n", response_ref->Status_code);
                printf("before_connect_time_microsec=%lld,after_response_time_microsec=%lld,seconds to connect=%lf,ttfb=%lf,total=%lf.total2=%lld\n", response_ref->Before_connect_time_microsec, response_ref->After_response_time_microsec, connect / 1e6, start / 1e6, total / 1e6, response_ref->After_response_time_microsec - response_ref->Before_connect_time_microsec);
            }
           if (response_ref->Debug > 3)
            {
                printf("%s\n%s\n", response_ref->Response_header, response_ref->Response_body);
            }

            curl_multi_remove_handle(curl_handle, easy_handle);
            curl_easy_cleanup(easy_handle);

            break;
        }

        default:
        {
            fprintf(stderr, "CURLMSG default\n");
            break;
        }
        }
    }
}

static void curl_perform(uv_poll_t *req, int status, int events)
{
    CURLM * curl_handle=( CURLM *)req->loop->data;
    int running_handles;
    int flags = 0;
    curl_context_t *context;

    if (events & UV_READABLE)
        flags |= CURL_CSELECT_IN;
    if (events & UV_WRITABLE)
        flags |= CURL_CSELECT_OUT;

    context = (curl_context_t *)req->data;

    curl_multi_socket_action(curl_handle, context->sockfd, flags,
                                   &running_handles);

    on_request_complete(curl_handle);
}

static void on_timeout(uv_timer_t *req)
{
    CURLM * curl_handle=( CURLM *)req->loop->data;
    int running_handles;
    // printf("on_timeout curl_handle=%ld\n",(long)curl_handle);
    curl_multi_socket_action(curl_handle, CURL_SOCKET_TIMEOUT, 0,
                                   &running_handles);
    on_request_complete(curl_handle);
}

int api_req_async::start_timeout(CURLM *multi, long timeout_ms, void *userp)
{
    // printf("timeout_ms->%ld\n", timeout_ms);
    if (timeout_ms < 0)
    {
        uv_timer_stop(&timeout);
    }
    else
    {
        if (timeout_ms == 0)
            timeout_ms = 1; /* 0 means directly call socket_action, but we will do it
                               in a bit */
        uv_timer_start(&timeout, on_timeout, timeout_ms, 0);
    }
    return 0;
}


int api_req_async::handle_socket(CURL *easy, curl_socket_t s, int action, void *userp,
                                 void *socketp)
{
    curl_context_t *curl_context;
    int events = 0;

    switch (action)
    {
    case CURL_POLL_IN:
    case CURL_POLL_OUT:
    case CURL_POLL_INOUT:
        curl_context = socketp ? (curl_context_t *)socketp : create_curl_context(s);

        curl_multi_assign(curl_handle, s, (void *)curl_context);

        if (action != CURL_POLL_IN)
            events |= UV_WRITABLE;
        if (action != CURL_POLL_OUT)
            events |= UV_READABLE;

        uv_poll_start(&curl_context->poll_handle, events, curl_perform);
        break;
    case CURL_POLL_REMOVE:
        if (socketp)
        {
            uv_poll_stop(&((curl_context_t *)socketp)->poll_handle);
            destroy_curl_context((curl_context_t *)socketp);
            curl_multi_assign(curl_handle, s, NULL);
        }
        break;
    default:
        abort();
    }

    return 0;
}




api_req_async::api_req_async(int th_id)
{
    // printf("api_req_async=%d\n",th_id);
    thread_id = th_id;
    // curl_handle = multi_handler;
    // loop = uv_loop_new();
}

api_req_async::~api_req_async()
{
    // uv_loop_delete(loop);
    // free(loop);
    // free(curl_handle);
}

void *api_req_async::get_result()
{
    thread_data *td = (thread_data *)data;
    // printf("%d,%d\n",td->th_pool_data.start_index,td->th_pool_data.end_index);

    return td;
}


struct Closure_handle_socket
{
    template <typename Any, typename RETURN_TYPE>
    static Any lambda_ptr_exec(CURL *easy, curl_socket_t s, int action, void *userp, void *socketp)
    {
        return (Any)(*(RETURN_TYPE *)callback<RETURN_TYPE>())(easy, s, action, userp, socketp);
    }

    template <typename Any = void, typename CALLER_TYPE = Any (*)(CURL *, curl_socket_t, int, void *, void *), typename RETURN_TYPE>
    static CALLER_TYPE create(RETURN_TYPE &t)
    {
        callback<RETURN_TYPE>(&t);
        return (CALLER_TYPE)lambda_ptr_exec<Any, RETURN_TYPE>;
    }

    template <typename T>
    static void *callback(void *new_callback = nullptr)
    {
        static void *callback;
        if (new_callback != nullptr)
            callback = new_callback;
        return callback;
    }
};

struct Closure_start_timeout
{
    template <typename Any, typename RETURN_TYPE>
    static Any lambda_ptr_exec(CURLM *multi, long timeout_ms, void *userp)
    {
        return (Any)(*(RETURN_TYPE *)callback<RETURN_TYPE>())(multi, timeout_ms, userp);
    }

    template <typename Any = void, typename CALLER_TYPE = Any (*)(CURLM *, long, void *), typename RETURN_TYPE>
    static CALLER_TYPE create(RETURN_TYPE &t)
    {
        callback<RETURN_TYPE>(&t);
        return (CALLER_TYPE)lambda_ptr_exec<Any, RETURN_TYPE>;
    }

    template <typename T>
    static void *callback(void *new_callback = nullptr)
    {
        static void *callback;
        if (new_callback != nullptr)
            callback = new_callback;
        return callback;
    }
};


void *api_req_async::run(void *data)
{
    // loop = create_loop();
    // loop = uv_loop_new();
    loop = uv_loop_new();
    curl_handle = curl_multi_init();
    loop->data = curl_handle;
    uv_timer_init(loop, &timeout);

    // curl_global_cleanup();
    if (curl_global_init(CURL_GLOBAL_ALL))
    {
        fprintf(stderr, "Could not init curl\n");
        loop = NULL;
        return NULL;
    }

    printf("1-curl_handle=%ld,loop-%ld\n", (long)curl_handle, (long)loop);

    this->data = data;

    loop_addrs_int = (long)loop;

    // curl_handle = curl_multi_init();

    auto handle_socket_with_context = [=](CURL *easy, curl_socket_t s, int action, void *userp, void *socketp) -> int
    {
        return handle_socket(easy, s, action, userp, socketp);
    };
    auto _closure_handle_socket = Closure_handle_socket::create<int>(handle_socket_with_context);
    auto start_timeout_with_context = [=](CURLM *multi, long timeout_ms, void *userp) -> int
    {
        return start_timeout(multi, timeout_ms, userp);
    };
    auto _closure_start_timeout = Closure_start_timeout::create<int>(start_timeout_with_context);

    curl_multi_setopt(curl_handle, CURLMOPT_SOCKETFUNCTION, _closure_handle_socket);
    curl_multi_setopt(curl_handle, CURLMOPT_TIMERFUNCTION, _closure_start_timeout);

    if (loop == NULL)
    {
        return NULL;
    }
    thread_data *td = (thread_data *)data;

    curl_multi_setopt(curl_handle, CURLMOPT_MAX_TOTAL_CONNECTIONS, (td->th_pool_data.end_index - td->th_pool_data.start_index + 1));
    for (int i = td->th_pool_data.start_index; i <= td->th_pool_data.end_index; i++)
    {
        add_request_to_event_loop(&(td->req_inputs_ptr[i]), &(td->response_ref_ptr[i]), td->debug_flag);
    }
    uv_run(loop, UV_RUN_DEFAULT);

    curl_multi_cleanup(curl_handle);
    return NULL;
}