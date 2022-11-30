
#include "api_req_async.hpp"

using namespace std;

long long get_current_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec) * 1e6) + (tv.tv_usec);
}

map<long, api_req_async *> threadid_class_map;

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

api_req_async::api_req_async(int th_id)
{
    // printf("api_req_async=%d\n",th_id);
    thread_id = th_id;
    // loop = uv_default_loop();
    // loop = uv_loop_new();
    // printf("loop=%d\n", loop);

    // if (curl_global_init(CURL_GLOBAL_ALL))
    // {
    //     fprintf(stderr, "Could not init curl\n");
    //     loop = NULL;
    //     return;
    // }

    // uv_timer_init(loop, &timeout);

    // curl_handle = curl_multi_init();

    // auto handle_socket_with_context = [=](CURL *easy, curl_socket_t s, int action, void *userp, void *socketp) -> int
    // {
    //     return handle_socket(easy, s, action, userp, socketp);
    // };
    // auto _closure_handle_socket = Closure_handle_socket::create<int>(handle_socket_with_context);
    // auto start_timeout_with_context = [=](CURLM *multi, long timeout_ms, void *userp) -> int
    // {
    //     return start_timeout(multi, timeout_ms, userp);
    // };
    // auto _closure_start_timeout = Closure_start_timeout::create<int>(start_timeout_with_context);

    // curl_multi_setopt(curl_handle, CURLMOPT_SOCKETFUNCTION, _closure_handle_socket);
    // curl_multi_setopt(curl_handle, CURLMOPT_TIMERFUNCTION, _closure_start_timeout);
}

api_req_async::~api_req_async()
{
    // uv_loop_delete(loop);
    free(loop);
    free(curl_handle);
}

void *api_req_async::run(void *data)
{
    this->on_timeout_ptr = &api_req_async::on_timeout;

    loop = uv_loop_new();
    loop_addrs_int = (long)loop;
    // printf("loop=%ld\n", loop_addrs_int);
    threadid_class_map[loop_addrs_int] = this;

    if (curl_global_init(CURL_GLOBAL_ALL))
    {
        printf("Could not init curl\n");
        loop = NULL;
        return NULL;
    }

    uv_timer_init(loop, &timeout);

    curl_handle = curl_multi_init();

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

    for (int i = td->th_pool_data.start_index; i <= td->th_pool_data.end_index; i++)
    {
        add_request_to_event_loop(&(td->req_inputs_ptr[i]), &(td->response_ref_ptr[i]), td->debug_flag);
    }
    uv_run(loop, UV_RUN_DEFAULT);
    curl_multi_cleanup(curl_handle);
    return NULL;
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

void api_req_async::curl_close_cb(uv_handle_t *handle)
{
    curl_context_t *context = (curl_context_t *)handle->data;
    free(context);
}

struct Closure_curl_close_cb
{
    template <typename Any, typename RETURN_TYPE>
    static Any lambda_ptr_exec(uv_handle_t *handle)
    {
        return (Any)(*(RETURN_TYPE *)callback<RETURN_TYPE>())(handle);
    }

    template <typename Any = void, typename CALLER_TYPE = Any (*)(uv_handle_t *handle), typename RETURN_TYPE>
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

void api_req_async::destroy_curl_context(curl_context_t *context)
{
    auto curl_close_cb_with_context = [=](uv_handle_t *handle)
    {
        return curl_close_cb(handle);
    };
    auto _curl_close_cb_with_context = Closure_curl_close_cb::create<void>(curl_close_cb_with_context);
    uv_close((uv_handle_t *)&context->poll_handle, (uv_close_cb)_curl_close_cb_with_context);
}

void api_req_async::curl_perform(uv_poll_t *req, int status, int events)
{
    int running_handles;
    int flags = 0;
    curl_context_t *context;
    CURLMcode res;

    if (events & UV_READABLE)
        flags |= CURL_CSELECT_IN;
    if (events & UV_WRITABLE)
        flags |= CURL_CSELECT_OUT;

    context = (curl_context_t *)req->data;

    res = curl_multi_socket_action(curl_handle, context->sockfd, flags,
                                   &running_handles);

    on_request_complete();
}

void api_req_async::on_timeout(uv_timer_t *req)
{
    // printf("---on_timeout---\n");
    // printf("on_timeout-thread_id=%d\n", thread_id);
    // printf("on_timeout-thread_id=%d curl_handle=%p\n",thread_id,curl_handle);
    int running_handles;
    CURLMcode res;
    res = curl_multi_socket_action(curl_handle, CURL_SOCKET_TIMEOUT, 0,
                                   &running_handles);
    on_request_complete();
}

struct Closure_on_timeout
{
    template <typename Any, typename RETURN_TYPE>
    static Any lambda_ptr_exec(uv_timer_t *req)
    {
        return (Any)(*(RETURN_TYPE *)callback<RETURN_TYPE>())(req);
    }

    template <typename Any = void, typename CALLER_TYPE = Any (*)(uv_timer_t *req), typename RETURN_TYPE>
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

int api_req_async::start_timeout(CURLM *multi, long timeout_ms, void *userp)
{
    // printf("timeout_ms->%ld,%ln\n", timeout_ms,&timeout_ms);

    threadid_class_map[loop_addrs_int] = this;
    // printf("on_timeout_ptr=%ld,%d,%p\n", loop_addrs_int, thread_id, threadid_class_map[loop_addrs_int]);
    // // printf("on_timeout-%d curl_handle=%d,%p\n",(long)thread_id,this->curl_handle,this);
    auto on_timeout_with_context = [=](uv_timer_t *req)
    {
        // printf("on_timeout_ptr2=%ld\n", (long)req->data);
        api_req_async *_this = threadid_class_map[(long)req->loop];
        // printf("on_timeout2-%d,%p\n", _this->thread_id, _this);
        return  _this->on_timeout(req);
    };
    auto _on_timeout_with_context = Closure_on_timeout::create<void>(on_timeout_with_context);

    if (timeout_ms < 0)
    {
        uv_timer_stop(&timeout);
    }
    else
    {
        if (timeout_ms == 0)
            timeout_ms = 1; /* 0 means directly call socket_action, but we will do it
                               in a bit */
        // printf("uv_timer_start-%d curl_handle=%ld,%p,%p\n",thread_id,loop_addrs_int,this->curl_handle,this);
        uv_timer_start(&timeout, _on_timeout_with_context, timeout_ms, 0);
    }
    return 0;
}

struct Closure_curl_perform
{
    template <typename Any, typename RETURN_TYPE>
    static Any lambda_ptr_exec(uv_poll_t *req, int status, int events)
    {
        return (Any)(*(RETURN_TYPE *)callback<RETURN_TYPE>())(req, status, events);
    }

    template <typename Any = void, typename CALLER_TYPE = Any (*)(uv_poll_t *req, int status, int events), typename RETURN_TYPE>
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

int api_req_async::handle_socket(CURL *easy, curl_socket_t s, int action, void *userp,
                                 void *socketp)
{
    // printf("handle_socket\n");
    auto curl_perform_with_context = [=](uv_poll_t *req, int status, int events)
    {
        return curl_perform(req, status, events);
    };
    auto _curl_perform_with_context = Closure_curl_perform::create<void>(curl_perform_with_context);

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

        uv_poll_start(&curl_context->poll_handle, events, (uv_poll_cb)_curl_perform_with_context);
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

void api_req_async::add_request_to_event_loop(request_input *req_input, response_data *response_ref, int debug)
{
    response_ref->status_code = -2;
    response_ref->debug = debug;
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
    curl_multi_add_handle(curl_handle, curl);
    response_ref->before_connect_time_microsec = get_current_time();
    if (debug > 1)
    {
        printf("request added to event loop: %s\n", req_input->url);
    }
}

void api_req_async::on_request_complete()
{

    printf("th_id=%d,loop_id=%ld\n", thread_id,loop_addrs_int);

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
            printf("---done_url=%s\n", done_url);
            curl_easy_getinfo(easy_handle, CURLINFO_PRIVATE, &response_ref);
            // if (res != CURLE_OK)
            // {
            //     response_ref->status_code = -1;
            //     response_ref->err_code = res;
            // }
            response_ref->after_response_time_microsec = get_current_time();

            curl_off_t start = -1, connect = -1, total = -1;
            struct memory body = {0}, header = {0};
            // from response
            curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, response_writer);
            curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, (void *)&body);
            curl_easy_setopt(easy_handle, CURLOPT_HEADERDATA, &header);
            curl_easy_setopt(easy_handle, CURLOPT_HEADERFUNCTION, response_writer);
            printf("%s DONE\n", done_url);

            curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &response_ref->status_code);
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

            if (response_ref->debug > 2)
            {
                printf("status_code=%d\n", response_ref->status_code);
                printf("before_connect_time_microsec=%lld,after_response_time_microsec=%lld,seconds to connect=%lf,ttfb=%lf,total=%lf.total2=%lld\n", response_ref->before_connect_time_microsec, response_ref->after_response_time_microsec, connect / 1e6, start / 1e6, total / 1e6, response_ref->after_response_time_microsec - response_ref->before_connect_time_microsec);
            }
            if (response_ref->debug > 3)
            {
                printf("%s\n%s\n", header.data, body.data);
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
