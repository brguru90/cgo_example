#include "api_req.h"

// git clone https://github.com/curl/curl.git
// https://gist.github.com/nolim1t/126991/ae3a7d36470d2a81190339fbc78821076a4059f7
// https://github.com/ppelleti/https-example/blob/master/https-client.c
// https://stackoverflow.com/questions/40303354/how-to-make-an-https-request-in-c
// https://stackoverflow.com/questions/22077802/simple-c-example-of-doing-an-http-post-and-consuming-the-response
// https://stackoverflow.com/questions/62387069/golang-parse-raw-http-2-response
// https://curl.se/libcurl/c/sendrecv.html

#define NUM_THREAD 5

void *goCallback_wrap(void *vargp)
{
    int *myid = (int *)vargp;
    printf("tid=%d\n", *myid);
    // goCallback(*myid);
    pthread_exit(NULL);
}

void run_bulk_api_request(char *s)
{
    printf("%s\n", s);

    int i, nor_of_thread;
    // pthread_t threads[NUM_THREAD];
    printf("Enter number of thread\n");
    scanf("%d", &nor_of_thread);
    printf("Entered %d\n", nor_of_thread);
    pthread_t *threads = malloc(sizeof(pthread_t) * nor_of_thread);
    pthread_t tid;

    for (i = 0; i < nor_of_thread; i++)
    {
        pthread_create(&threads[i], NULL, goCallback_wrap, (void *)&threads[i]);
    }

    for (i = 0; i < nor_of_thread; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // printf("Main thread exiting...\n");
    // pthread_exit(NULL);

    goCallback(-1);
}

long long get_current_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

static size_t response_writer(void *data, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct memory *mem = (struct memory *)userp;

    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (ptr == NULL)
        return 0; /* out of memory! */

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), data, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;

    return realsize;
}

void send_raw_request(request_input *req_input, response_data *response_ref, int debug)
{
    response_data res_data;
    res_data.status_code = -2;
    if (debug > 0)
    {
        printf("%s\n", req_input->url);
        printf("header count=%d\n\n", req_input->headers_len);
        printf("%s\n\n", req_input->headers[0].header);
        printf("body=%s\n\n", req_input->body);
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
        curl_easy_setopt(curl, CURLOPT_VERBOSE, (long)debug);
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

        res_data.before_connect_time = get_current_time();
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
        if (debug > 0)
        {
            printf("status_code=%ld\n", response_code);
            printf("connect=%lf,ttfb=%lf,total=%lf\n", connect / 1e6, start / 1e6, total / 1e6);
            printf("%s\n%s\n", header.data, body.data);
        }

        res_data.status_code = response_code;
        res_data.connect_time_microsec = connect;
        res_data.time_at_first_byte_microsec = start;
        res_data.total_time_microsec = total;
        res_data.response_header = header.data;
        res_data.response_body = body.data;

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    *response_ref = res_data;
}

void *send_raw_request_wrap_for_thread(void *data)
{
    thread_data *td = (thread_data *)data;
    send_raw_request(td->req_res.req_inputs_ptr,td->req_res.response_ref_ptr,*td->req_res.debug_flag);
}

void send_request_concurrently(request_input *req_inputs, response_data *response_ref,int total_requests, int debug)
{

    int i;
    pthread_t *threads = malloc(sizeof(pthread_t) * total_requests);
    thread_data *threads_data = malloc(sizeof(thread_data) * total_requests);

    for (i = 0; i < total_requests; i++)
    {
        threads_data[i].req_res.req_inputs_ptr = &req_inputs[i];
        threads_data[i].req_res.response_ref_ptr = &response_ref[i];
        threads_data[i].req_res.debug_flag = &debug;
        threads_data->thread_id = i;
    }

    for (i = 0; i < total_requests; i++)
    {
        pthread_create(&threads[i], NULL, send_raw_request_wrap_for_thread, (void *)&threads_data[i]);
    }

    for (i = 0; i < total_requests; i++)
    {
        pthread_join(threads[i], NULL);
    }
}