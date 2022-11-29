#include "api_req.h"
#include "api_req.hpp"

// git clone https://github.com/curl/curl.git
// https://gist.github.com/nolim1t/126991/ae3a7d36470d2a81190339fbc78821076a4059f7
// https://github.com/ppelleti/https-example/blob/master/https-client.c
// https://stackoverflow.com/questions/40303354/how-to-make-an-https-request-in-c
// https://stackoverflow.com/questions/22077802/simple-c-example-of-doing-an-http-post-and-consuming-the-response
// https://stackoverflow.com/questions/62387069/golang-parse-raw-http-2-response
// https://curl.se/libcurl/c/sendrecv.html

#ifdef _WIN32
#include <Windows.h>
struct timezone
{
    int tz_minuteswest;
    int tz_dsttime;
};

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    if (tv)
    {
        FILETIME filetime; /* 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 00:00 UTC */
        ULARGE_INTEGER x;
        ULONGLONG usec;
        static const ULONGLONG epoch_offset_us = 11644473600000000ULL; /* microseconds betweeen Jan 1,1601 and Jan 1,1970 */

#if _WIN32_WINNT >= _WIN32_WINNT_WIN8
        GetSystemTimePreciseAsFileTime(&filetime);
#else
        GetSystemTimeAsFileTime(&filetime);
#endif
        x.LowPart = filetime.dwLowDateTime;
        x.HighPart = filetime.dwHighDateTime;
        usec = x.QuadPart / 10 - epoch_offset_us;
        tv->tv_sec = (time_t)(usec / 1000000ULL);
        tv->tv_usec = (long)(usec % 1000000ULL);
    }
    if (tz)
    {
        TIME_ZONE_INFORMATION timezone;
        GetTimeZoneInformation(&timezone);
        tz->tz_minuteswest = timezone.Bias;
        tz->tz_dsttime = 0;
    }
    return 0;
}
#endif

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

// void send_raw_request(request_input *req_input, response_data *response_ref, int debug)
// {
//     response_data res_data;
//     res_data.status_code = -2;

//     switch (debug)
//     {
//     case 2:
//         printf("header count=%d\n\n", req_input->headers_len);
//         printf("%s\n\n", req_input->headers[0].header);
//         printf("body=%s\n\n", req_input->body);
//     case 1:
//         printf("debug_level%d\n", debug);
//         printf("%s\n", req_input->url);
//         break;

//     default:
//         break;
//     }

//     CURL *curl;
//     CURLcode res;
//     curl_global_init(CURL_GLOBAL_ALL);
//     curl = curl_easy_init();
//     struct curl_slist *header_list = NULL;
//     if (req_input->headers_len > 0)
//     {
//         for (int i = 0; i < req_input->headers_len; i++)
//         {
//             curl_slist_append(header_list, req_input->headers[i].header);
//         }
//     }
//     // struct curl_slist *header_list = curl_slist_append(NULL, "Content-Type: text/html");
//     if (curl)
//     {
//         long response_code;
//         curl_off_t start = -1, connect = -1, total = -1;
//         struct memory body = {0}, header = {0};
//         // to request
//         curl_easy_setopt(curl, CURLOPT_VERBOSE, debug > 2 ? 1L : 0);
//         curl_easy_setopt(curl, CURLOPT_URL, req_input->url);
//         curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, 0);
//         curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
//         curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
//         curl_easy_setopt(curl, CURLOPT_USERAGENT, "cgo benchmark tool");
//         curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req_input->body);
//         curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, req_input->method);

//         // from response
//         curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, response_writer);
//         curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&body);
//         curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header);
//         curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, response_writer);

//         // need to map all inputs like
//         // Header
//         // query param
//         // method
//         // body
//         // blob files
//         // keep alive
//         // time out
//         // chunked, see?
//         // UA

//         res_data.before_connect_time_microsec = get_current_time();
//         res = curl_easy_perform(curl);
//         /* Check for errors */
//         if (res != CURLE_OK)
//         {
//             if (debug > 1)
//             {
//                 fprintf(stderr, "curl_easy_perform() failed: %s\n",
//                         curl_easy_strerror(res));
//             }
//             response_code = -1;
//         }
//         curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
//         res = curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME_T, &connect);
//         if (CURLE_OK != res)
//         {
//             connect = -1;
//         }
//         res = curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME_T, &start);
//         if (CURLE_OK != res)
//         {
//             start = -1;
//         }
//         res = curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &total);
//         if (CURLE_OK != res)
//         {
//             total = -1;
//         }
//         if (debug > 1)
//         {
//             printf("status_code=%ld\n", response_code);
//             printf("before_connect_time_microsec=%lld,seconds to connect=%lf,ttfb=%lf,total=%lf\n",res_data.before_connect_time_microsec, connect / 1e6, start / 1e6, total / 1e6);
//         }
//         if (debug > 2)
//         {
//             printf("%s\n%s\n", header.data, body.data);
//         }

//         res_data.status_code = response_code;
//         res_data.connect_time_microsec = connect;
//         res_data.time_at_first_byte_microsec = start;
//         res_data.total_time_microsec = total;
//         res_data.connected_at_microsec=res_data.before_connect_time_microsec+connect;
//         res_data.first_byte_at_microsec=res_data.before_connect_time_microsec+start;
//         res_data.finish_at_microsec=res_data.before_connect_time_microsec+total;

//         res_data.response_header = header.data;
//         res_data.response_body = body.data;

//         curl_easy_cleanup(curl);
//     }
//     curl_global_cleanup();
//     *response_ref = res_data;
// }
void send_raw_request(request_input *req_input, response_data *response_ref, int debug)
{
    response_data res_data;
    res_data.status_code = -2;

    switch (debug)
    {
    case 2:
        printf("header count=%d\n\n", req_input->headers_len);
        printf("%s\n\n", req_input->headers[0].header);
        printf("body=%s\n\n", req_input->body);
    case 1:
        printf("debug_level%d\n", debug);
        printf("%s\n", req_input->url);
        break;

    default:
        break;
    }

    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    struct curl_slist *header_list = NULL;
    if (req_input->headers_len > 0)
    {
        for (int i = 0; i < req_input->headers_len; i++)
        {
            curl_slist_append(header_list, req_input->headers[i].header);
        }
    }
    // struct curl_slist *header_list = curl_slist_append(NULL, "Content-Type: text/html");
    if (curl)
    {
        long response_code;
        curl_off_t start = -1, connect = -1, total = -1;
        struct memory body = {0}, header = {0};
        // to request
        curl_easy_setopt(curl, CURLOPT_VERBOSE, debug > 2 ? 1L : 0);
        curl_easy_setopt(curl, CURLOPT_URL, req_input->url);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, 0);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "cgo benchmark tool");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req_input->body);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, req_input->method);

        // from response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, response_writer);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&body);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, response_writer);

        // need to map all inputs like
        // Header
        // query param
        // method
        // body
        // blob files
        // keep alive
        // time out
        // chunked, see?
        // UA

        res_data.before_connect_time_microsec = get_current_time();
        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
        {
            if (debug > 1)
            {
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                        curl_easy_strerror(res));
            }
            response_code = -1;
        }
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        res = curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME_T, &connect);
        if (CURLE_OK != res)
        {
            connect = -1;
        }
        res = curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME_T, &start);
        if (CURLE_OK != res)
        {
            start = -1;
        }
        res = curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &total);
        if (CURLE_OK != res)
        {
            total = -1;
        }
        if (debug > 1)
        {
            printf("status_code=%ld\n", response_code);
            printf("before_connect_time_microsec=%lld,seconds to connect=%lf,ttfb=%lf,total=%lf\n", res_data.before_connect_time_microsec, connect / 1e6, start / 1e6, total / 1e6);
        }
        if (debug > 2)
        {
            printf("%s\n%s\n", header.data, body.data);
        }

        res_data.status_code = response_code;
        res_data.connect_time_microsec = connect;
        res_data.time_to_first_byte_microsec = start;
        res_data.total_time_from_curl_microsec = total;
        res_data.total_time_microsec = (res_data.after_response_time_microsec - res_data.before_connect_time_microsec);
        res_data.connected_at_microsec = res_data.before_connect_time_microsec + connect;
        res_data.first_byte_at_microsec = res_data.before_connect_time_microsec + start;
        res_data.finish_at_microsec = res_data.before_connect_time_microsec + res_data.total_time_microsec;

        res_data.response_header = header.data;
        res_data.response_body = body.data;

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    *response_ref = res_data;
}

// uv_loop_t *loop;
// CURLM *curl_handle;
// uv_timer_t timeout;

// curl_handlers_t *curl_handlers;

static curl_context_t *create_curl_context(curl_handlers_t curl_handlers, curl_socket_t sockfd)
{
    curl_context_t *context;

    context = (curl_context_t *)malloc(sizeof(*context));

    context->sockfd = sockfd;

    uv_poll_init_socket(curl_handlers.loop, &context->poll_handle, sockfd);
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

static void add_request_to_event_loop(curl_handlers_t curl_handlers, request_input *req_input, response_data *response_ref, int debug)
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

    curl_multi_add_handle(curl_handlers.curl_handle, curl);
    response_ref->before_connect_time_microsec = get_current_time();
    if (debug > 1)
    {
        printf("request added to event loop: %s\n", req_input->url);
    }
}

static void on_request_complete(curl_handlers_t curl_handlers, CURLMcode res)
{
    char *done_url;
    CURLMsg *message;
    int pending;
    CURL *easy_handle;
    response_data *response_ref;

    while ((message = curl_multi_info_read(curl_handlers.curl_handle, &pending)))
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
            CURLcode res2;
            res2 = curl_easy_getinfo(easy_handle, CURLINFO_CONNECT_TIME_T, &connect);
            if (CURLE_OK != res2)
            {
                connect = -1;
            }
            res2 = curl_easy_getinfo(easy_handle, CURLINFO_STARTTRANSFER_TIME_T, &start);
            if (CURLE_OK != res2)
            {
                start = -1;
            }
            res2 = curl_easy_getinfo(easy_handle, CURLINFO_TOTAL_TIME_T, &total);
            if (CURLE_OK != res2)
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

            curl_multi_remove_handle(curl_handlers.curl_handle, easy_handle);
            curl_easy_cleanup(easy_handle);

            break;
        }

        default:
            fprintf(stderr, "CURLMSG default\n");
            break;
        }
    }
}

static void curl_perform(curl_handlers_t curl_handlers, uv_poll_t *req, int status, int events)
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

    res = curl_multi_socket_action(curl_handlers.curl_handle, context->sockfd, flags,
                                   &running_handles);
    on_request_complete(curl_handlers, res);
}

static void on_timeout(curl_handlers_t curl_handlers, uv_timer_t *req)
{
    int running_handles;
    CURLMcode res;
    res = curl_multi_socket_action(curl_handlers.curl_handle, CURL_SOCKET_TIMEOUT, 0,
                                   &running_handles);
    on_request_complete(curl_handlers, res);
}

// std::function<void(uv_timer_t *req)> get_on_timeout_with_context(curl_handlers_t curl_handlers)
// {
//     return [=](uv_timer_t *req) mutable {
//         on_timeout(curl_handlers, req);
//     };
// }

static int start_timeout(curl_handlers_t curl_handlers, CURLM *multi, long timeout_ms, void *userp)
{
    printf("timeout_ms->%ld\n", timeout_ms);
    //
    // struct create_on_timeout_with_context
    // {
    //     create_on_timeout_with_context(curl_handlers_t curl_handlers) : ch(curl_handlers) {} // Constructor
    //     uv_timer_cb operator()(uv_timer_t *req) const { on_timeout(ch,req); }

    // private:
    //     curl_handlers_t ch;
    // };

    // create_on_timeout_with_context on_timeout_with_context(curl_handlers);

    // auto on_timeout_with_context = [curl_handlers](uv_timer_t *req)
    // {
    //     on_timeout(curl_handlers, req);
    // };

    // int(decltype(on_timeout_with_context)::*ptr)(uv_timer_t *req)const = &decltype(on_timeout_with_context)::operator();

    // struct create_on_timeout_with_context
    // {
    //     curl_handlers_t curl_handlers;
    //     void operator()(uv_timer_t *req) { on_timeout(curl_handlers, req); }
    // };

    // create_on_timeout_with_context on_timeout_with_context;
    // on_timeout_with_context.curl_handlers=curl_handlers;

    // https://www.nextptr.com/tutorial/ta1188594113/passing-cplusplus-captureless-lambda-as-function-pointer-to-c-api

    auto on_timeout_with_context = new std::function<void(uv_timer_t * req)>([=](uv_timer_t *req)
                                                                             { 

                                                                                 printf("------------");
                                                                                //  on_timeout(curl_handlers, req);
                                                                              });

    if (timeout_ms < 0)
    {
        uv_timer_stop(&curl_handlers.timeout);
    }
    else
    {
        if (timeout_ms == 0)
            timeout_ms = 1; /* 0 means directly call socket_action, but we will do it
                               in a bit */

        uv_timer_start(&curl_handlers.timeout, (uv_timer_cb)on_timeout_with_context, timeout_ms, 0);
    }
    return 0;
}

static int handle_socket(curl_handlers_t curl_handlers, CURL *easy, curl_socket_t s, int action, void *userp,
                         void *socketp)
{
    curl_context_t *curl_context;
    int events = 0;

    switch (action)
    {
    case CURL_POLL_IN:
    case CURL_POLL_OUT:
    case CURL_POLL_INOUT:
    {
        curl_context = socketp ? (curl_context_t *)socketp : create_curl_context(curl_handlers, s);

        curl_multi_assign(curl_handlers.curl_handle, s, (void *)curl_context);

        if (action != CURL_POLL_IN)
            events |= UV_WRITABLE;
        if (action != CURL_POLL_OUT)
            events |= UV_READABLE;

        // auto curl_perform_with_context = [curl_handlers](uv_poll_t *req, int status, int events)
        // {
        //     return curl_perform(curl_handlers, req, status, events);
        // };
        auto curl_perform_with_context = new std::function<void(uv_poll_t * req, int status, int events)>([=](uv_poll_t *req, int status, int events)
                                                                                                          { curl_perform(curl_handlers, req, status, events); });
        uv_poll_start(&curl_context->poll_handle, events, (uv_poll_cb)curl_perform_with_context);
        break;
    }
    case CURL_POLL_REMOVE:
    {
        if (socketp)
        {
            uv_poll_stop(&((curl_context_t *)socketp)->poll_handle);
            destroy_curl_context((curl_context_t *)socketp);
            curl_multi_assign(curl_handlers.curl_handle, s, NULL);
        }
        break;
    }
    default:
        abort();
    }

    return 0;
}

void *loop_on_the_thread(void *data)
{
    thread_data *td = (thread_data *)data;

    curl_handlers_t curl_handlers;

    curl_handlers.loop = uv_default_loop();

    if (curl_global_init(CURL_GLOBAL_ALL))
    {
        fprintf(stderr, "Could not init curl\n");
        return NULL;
    }

    uv_timer_init(curl_handlers.loop, &curl_handlers.timeout);

    curl_handlers.curl_handle = curl_multi_init();
    auto handle_socket_with_context = [curl_handlers](CURL *easy, curl_socket_t s, int action, void *userp, void *socketp) -> int
    {
        return handle_socket(curl_handlers, easy, s, action, userp, socketp);
    };
    auto start_timeout_with_context = [curl_handlers](CURLM *multi, long timeout_ms, void *userp) -> int
    {
        return start_timeout(curl_handlers, multi, timeout_ms, userp);
    };
    curl_multi_setopt(curl_handlers.curl_handle, CURLMOPT_SOCKETFUNCTION, handle_socket_with_context);
    curl_multi_setopt(curl_handlers.curl_handle, CURLMOPT_TIMERFUNCTION, start_timeout_with_context);
    for (int i = td->th_pool_data.start_index; i <= td->th_pool_data.end_index; i++)
    {

        add_request_to_event_loop(curl_handlers, &(td->req_inputs_ptr[i]), &(td->response_ref_ptr[i]), td->debug_flag);
        // send_raw_request(&(td->req_inputs_ptr[i]), &(td->response_ref_ptr[i]), td->debug_flag);
    }
    uv_run(curl_handlers.loop, UV_RUN_DEFAULT);
    curl_multi_cleanup(curl_handlers.curl_handle);
    return NULL;
}

void send_request_in_concurrently(request_input *req_inputs, response_data *response_ref, int total_requests, int total_threads, int debug)
{

    int num_of_threads = total_requests >= total_threads ? total_threads : total_requests;
    int max_work_on_thread = floor((float)total_requests / num_of_threads);
    int left_out_work = total_requests % num_of_threads;

    printf("total_requests=%d,total_threads=%d,num_of_threads=%d,max_work_on_thread=%d,left_out_work=%d\n", total_requests, total_threads, num_of_threads, max_work_on_thread, left_out_work);

    thread_pool_data proc_data[left_out_work == 0 ? num_of_threads : num_of_threads + 1];

    for (int p = 0; p < num_of_threads; p++)
    {
        proc_data[p].start_index = p * max_work_on_thread;
        proc_data[p].end_index = (proc_data[p].start_index + max_work_on_thread) - 1;
        proc_data[p].full_index = false;
    }
    if (left_out_work > 0)
    {
        proc_data[num_of_threads].start_index = num_of_threads * max_work_on_thread;
        proc_data[num_of_threads].end_index = (proc_data[num_of_threads].start_index + left_out_work) - 1;
        proc_data[num_of_threads].full_index = false;
    }

    int thread_size = (left_out_work == 0 ? num_of_threads : num_of_threads + 1);
    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_size);
    thread_data *threads_data = (thread_data *)malloc(sizeof(thread_data) * thread_size);

    for (int i = 0; i < thread_size; i++)
    {
        threads_data[i].req_inputs_ptr = req_inputs;
        threads_data[i].response_ref_ptr = response_ref;
        threads_data[i].debug_flag = debug;
        threads_data[i].thread_id = i;
        threads_data[i].th_pool_data = proc_data[i];
    }
    loop_on_the_thread((void *)&threads_data[0]);
    // for (int p = 0; p < thread_size; p++)
    // {
    //     if (pthread_create(&threads[p], NULL, loop_on_the_thread, (void *)&threads_data[p]) != 0)
    //     {
    //         perror("pthread_create() error");
    //         exit(1);
    //     }
    // }

    // for (int i = 0; i < thread_size; i++)
    // {
    //     pthread_join(threads[i], NULL);
    // }
    // free(threads);
    // free(threads_data);
}
