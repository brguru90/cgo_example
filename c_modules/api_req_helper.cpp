#include "api_req_async.hpp"

#define BUFFER_SIZE 100
#define PORT 14507
#define SA struct sockaddr

pthread_mutex_t lock;

char end_of_data[] = "end_of_data";

typedef void (*get_received_data_type)(StringType *raw_response);

void loop_on_the_thread(void *data)
{
    thread_data *td = (thread_data *)data;
    td->api_req_async_on_thread->run(td);
}

void on_exit(uv_process_t *req, int64_t exit_status, int term_signal)
{
    fprintf(stderr, "Process exited with status %" PRId64 ", signal %d\n", exit_status, term_signal);
    uv_close((uv_handle_t *)req, NULL);
}

// void response_data_to_json(thread_data td)
// {

//     struct_mapping::reg(&headers_type::header, "header");

//     struct_mapping::reg(&request_input::body, "body");
//     struct_mapping::reg(&request_input::cookies, "cookies");
//     struct_mapping::reg(&request_input::headers, "headers");
//     struct_mapping::reg(&request_input::headers_len, "headers_len");
//     struct_mapping::reg(&request_input::method, "method");
//     struct_mapping::reg(&request_input::time_out_in_sec, "time_out_in_sec");
//     struct_mapping::reg(&request_input::uid, "uid");
//     struct_mapping::reg(&request_input::url, "url");

//     struct_mapping::reg(&response_data::after_response_time_microsec, "after_response_time_microsec");
//     struct_mapping::reg(&response_data::before_connect_time_microsec, "before_connect_time_microsec");
//     struct_mapping::reg(&response_data::connect_time_microsec, "connect_time_microsec");
//     struct_mapping::reg(&response_data::connected_at_microsec, "connected_at_microsec");
//     struct_mapping::reg(&response_data::debug, "debug");
//     struct_mapping::reg(&response_data::err_code, "err_code");
//     struct_mapping::reg(&response_data::finish_at_microsec, "finish_at_microsec");
//     struct_mapping::reg(&response_data::first_byte_at_microsec, "first_byte_at_microsec");
//     struct_mapping::reg(&response_data::response_body, "response_body");
//     struct_mapping::reg(&response_data::response_header, "response_header");
//     struct_mapping::reg(&response_data::status_code, "status_code");
//     struct_mapping::reg(&response_data::time_to_first_byte_microsec, "time_to_first_byte_microsec");
//     struct_mapping::reg(&response_data::total_time_from_curl_microsec, "total_time_from_curl_microsec");
//     struct_mapping::reg(&response_data::total_time_microsec, "total_time_microsec");
//     struct_mapping::reg(&response_data::uid, "uid");

//     struct_mapping::reg(&thread_pool_data::end_index, "end_index");
//     struct_mapping::reg(&thread_pool_data::full_index, "full_index");
//     struct_mapping::reg(&thread_pool_data::pid, "pid");
//     struct_mapping::reg(&thread_pool_data::start_index, "start_index");
//     struct_mapping::reg(&thread_pool_data::uuid, "uuid");

//     struct_mapping::reg(&thread_data::req_inputs_ptr, "req_inputs_ptr");
//     struct_mapping::reg(&thread_data::api_req_async_on_thread, "api_req_async_on_thread");
//     struct_mapping::reg(&thread_data::debug_flag, "debug_flag");
//     struct_mapping::reg(&thread_data::response_ref_ptr, "response_ref_ptr");
//     struct_mapping::reg(&thread_data::th_pool_data, "th_pool_data");
//     struct_mapping::reg(&thread_data::thread_id, "thread_id");

//     std::ostringstream json_data;
//     struct_mapping::map_struct_to_json(td, json_data, "  ");
//     std::cout << json_data.str() << std::endl;
// }

void send_data(char* serialized)
{
    int _size = strlen(serialized) + strlen(end_of_data);
    char bytes[_size];
    memcpy(bytes, serialized, strlen(serialized));
    for (int i = strlen(serialized), j = 0; i < _size && j < strlen(end_of_data); i++, j++)
    {
        bytes[i] = end_of_data[j];
    }

    // printf("data to send2=%s,len=%ld\n", bytes, strlen(bytes));
    // for (int l = 0; l < strlen(bytes); l++)
    //     // printf("%02X ", bytes[l]);
    //      printf("%c", bytes[l]);
    // printf("\n");

    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");

    // function for chat
    // write(sockfd, buff, sizeof(buff));
    send(sockfd, bytes, sizeof(bytes), 0);
    // send(sockfd, end_of_data, sizeof(end_of_data), 0);
    bzero(bytes, strlen(bytes));

    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    // just make wait sender
    recv(connfd, buffer, BUFFER_SIZE - 1, 0);
    bzero(buffer, BUFFER_SIZE);

    // close the socket
    close(sockfd);
}

void my_strcpy(StringType &dest, char *src, int length)
{
    int prev_length = dest.length;
    char temp[prev_length];
    // resize & repopulate
    memcpy(&temp, dest.ch, prev_length);
    dest.ch = (char *)malloc(sizeof(char *) * (prev_length + length + 1));
    memcpy(dest.ch, &temp, prev_length);
    int j = prev_length;
    for (int i = 0; i < length; i++)
    {
        // printf("copy %02X ", dest.ch[prev_length + i]);
        dest.ch[j + i] = src[i];
    }
    dest.length = prev_length + length;
}

int isSubString(StringType &dest, char end_of_data[])
{
    int i = dest.length;
    // printf("i->%d,%d\n",i,i>=0);
    int end_len = strlen(end_of_data);
    while (i-- >= 0)
    {
        // printf("i->%d,%d\n",i,i>=0);
        if (dest.ch[i] == end_of_data[end_len - 1])
        {
            for (int j = end_len - 1; j >= 0 && i >= 0; j--)
            {
                if (dest.ch[i--] == end_of_data[j])
                {
                    if (j == 0)
                        return i + end_len + 1;
                }
                else
                {
                    break;
                }
            }
        }
    }
    return -1;
}

int *receive_data_sockfd;
void receive_data(int thread_size, get_received_data_type get_received_data_cb)
{
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;
    receive_data_sockfd = &sockfd;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0)
    {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);

    int i = thread_size;
    while (1)
    {
        // Accept the data packet from client and verification
        connfd = accept(sockfd, (SA *)&cli, (socklen_t *)&len);
        if (connfd < 0)
        {
            printf("server accept failed...\n");
            exit(0);
        }
        else
            printf("server accept the client...\n");

        // Function for chatting between client and server
        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);
        int n, cond;

        // char *raw_response = (char *)malloc(1);
        StringType raw_response;
        // raw_response.ch=(char*)malloc(1);
        raw_response.length = 0;
        raw_response.ch = (char *)"";
        while ((cond = recv(connfd, buffer, BUFFER_SIZE - 1, 0)) > 0)
        {
            // printf("%d)\n", cond);
            // printf("Received:\n");
            // for (int l = 0; l < sizeof(buffer); l++)
            //     printf("%02X ", buffer[l]);
            // printf("\n");
            // strcpy(raw_response, buffer);
            my_strcpy(raw_response, buffer, sizeof(buffer) - 1);

            // strncpy(raw_response, buffer,sizeof(buffer));
            // printf("raw_response=%s,size=%d\n", raw_response.ch, raw_response.length);
            // if ( strstr(raw_response.ch,end_of_data) != NULL)
            int pos = 0;
            if ((pos = isSubString(raw_response, end_of_data)) >= 0)
            {
                // printf("end_of_data,%d\n", pos);
                raw_response.length = pos;
                get_received_data_cb(&raw_response);
                raw_response.ch = (char *)"";
                raw_response.length = 0;
                send(connfd, end_of_data, strlen(end_of_data), 0);
                break;
            }
            bzero(buffer, BUFFER_SIZE);
        }
        // infinite loop for chat
        // char *raw_response = malloc(1);
        // for (;;)
        // {
        //     bzero(buff, MAX);

        //     // read the message from client and copy it in buffer
        //     read(connfd, buff, sizeof(buff));
        //     // print buffer which contains the client contents
        //     printf("From client: %s\t To client : ", buff);
        //     bzero(buff, MAX);
        //     n = 0;
        //     // copy server message in the buffer
        //     while ((buff[n++] = getchar()) != '\n')
        //         ;

        //     // and send that buffer to client
        //     write(connfd, buff, sizeof(buff));

        //     // if msg contains "Exit" then server exit and chat ended.
        //     if (strncmp("end_of_data", buff, 11) == 0)
        //     {
        //         printf("Server Exit...\n");
        //         break;
        //     }
        // }

        // After chatting close the socket
        close(connfd);
    }
    close(sockfd);
}

struct Closure_raw_response
{
    template <typename Any, typename RETURN_TYPE>
    static Any lambda_ptr_exec(StringType *raw_response)
    {
        return (Any)(*(RETURN_TYPE *)callback<RETURN_TYPE>())(raw_response);
    }

    template <typename Any = void, typename CALLER_TYPE = Any (*)(StringType *raw_response), typename RETURN_TYPE>
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

void update_response_data(int thread_size, response_data *response_ref)
{
    auto lamda = [&](StringType *raw_response) -> void
    {
        raw_response->length = raw_response->length - strlen(end_of_data);
        char tmp[raw_response->length];
        memcpy(tmp, raw_response->ch, raw_response->length);
        printf("\n\nfinal data from IPC->%ld,%s\n", raw_response->length, tmp);
        // for (int l = 0; l < raw_response->length; l++)
        //     printf("%02X ", raw_response->ch[l]);
        // printf("\n");

        // thread_data tmp;
        // memcpy(&tmp, raw_response->ch, sizeof(raw_response->length));

        // int start = tmp.th_pool_data.start_index;
        // int end = tmp.th_pool_data.end_index;

        // https://github.com/bk192077/struct_mapping/blob/master/example/struct_to_json/struct_to_json.cpp

        // printf("start=%d,end=%d,%d\n", start, end, tmp.thread_id);

        // for(int k=start;k<=end;k++){
        //     printf("status_code=%d\n",tmp.response_ref_ptr[k].status_code);
        // }
    };
    // auto *closure = new std::function<void(StringType *raw_response)>(lamda);
    auto lamda_with_context = Closure_raw_response::create<void>(lamda);

    receive_data(thread_size, (get_received_data_type)lamda_with_context);
}

typedef struct update_response_data_type
{
    int thread_size;
    response_data *response_ref;
} update_response_data_t;

void *run_update_response_data(void *data)
{
    update_response_data_type *td = (update_response_data_type *)data;
    update_response_data(td->thread_size, td->response_ref);
    return NULL;
}

void on_exit2(uv_process_t *req, int64_t exit_status, int term_signal)
{
    fprintf(stderr, "Process exited with status %" PRId64 ", signal %d\n", exit_status, term_signal);
    uv_close((uv_handle_t *)req, NULL);
}
struct child_worker
{
    uv_process_t req;
    uv_process_options_t options;
    uv_pipe_t pipe;
} * workers;

uv_loop_t *main_loop;
uv_process_t child_req;
uv_process_options_t options;

void create_process(int thread_size, int total_requests,uv_thread_t *threads, thread_data *threads_data, thread_pool_data proc_data[])
{

    // main_loop = uv_default_loop();

    // struct child_worker *workers = (struct child_worker *)calloc(thread_size, sizeof(struct child_worker));

    // for (int p = 0; p < thread_size; p++)
    // {
    //     struct child_worker *worker = &workers[p];
    //     uv_pipe_init(main_loop, &worker->pipe, 1);

    //     char *args[3];
    //     args[0] = (char*)"mkdir";
    //     args[1] = (char*)"test-dir";
    //     args[2] = NULL;

    //     uv_stdio_container_t child_stdio[3];
    //     child_stdio[0].flags = (uv_stdio_flags)(UV_CREATE_PIPE | UV_READABLE_PIPE);
    //     child_stdio[0].data.stream = (uv_stream_t *)&worker->pipe;
    //     child_stdio[1].flags = UV_IGNORE;
    //     child_stdio[2].flags = UV_INHERIT_FD;
    //     child_stdio[2].data.fd = 2;

    //     worker->options.stdio = child_stdio;
    //     worker->options.stdio_count = 3;

    //     worker->options.exit_cb = on_exit2;
    //     worker->options.file = "mkdir";
    //     worker->options.args = args;

    //     if ((proc_data[p].pid = uv_spawn(main_loop, &worker->req, &worker->options)) == 0)
    //     {
    //         if (proc_data[p].pid)
    //         {
    //             printf("failed to create process\n");
    //             fprintf(stderr, "%s\n", uv_strerror(proc_data[p].pid));
    //             exit(1);
    //         }
    //         loop_on_the_thread((void *)&threads_data[p]);
    //         thread_data td = (thread_data)threads_data[p];
    //         int start = td.th_pool_data.start_index;
    //         int end = td.th_pool_data.end_index;
    //         response_data *td_arr=(response_data*)malloc(sizeof(response_data)*(end-start+1));
    //         for(int k=start;k<=end;k++){
    //             // td_arr[k]=td.response_ref_ptr[k];
    //             printf("thread=%d,Status_code=>%d\n",td.thread_id,td.response_ref_ptr[k].Status_code);
    //             // printf("Response_header=%s\n",td.response_ref_ptr[k].Resp_header);
    //             // printf("Response_body=>%s\n",td_arr[k].Response_body);
    //         }

    //         // // thread_data_to_json(*td_arr,end-start+1);
    //         printf("td=%d\n", td.thread_id);
    //         // thread_data *td2 = (thread_data*)td.api_req_async_on_thread->get_result();

    //         // char bytes[sizeof(td)];
    //         // memcpy(bytes, &td, sizeof(td));

    //         // printf("data to send1=%s,len=%ld\n", bytes, sizeof(bytes));
    //         // for (int l = 0; l < sizeof(bytes); l++)
    //         //     printf("%02X ", bytes[l]);
    //         // printf("\n");

    //         // response_data_to_json(td);
    //         // send_data(td);
    //         // exit(0);
    //     }
    // }

    // uv_run(main_loop, UV_RUN_DEFAULT);

    for (int p = 0; p < thread_size; p++)
    {
        int temp_pid;
        if ((proc_data[p].pid = fork()) == 0)
        {
            if (proc_data[p].pid == -1)
            {
                printf("failed to create process\n");
                exit(1);
            }
            loop_on_the_thread((void *)&threads_data[p]);
            thread_data td = (thread_data)threads_data[p];
            int start = td.th_pool_data.start_index;
            int end = td.th_pool_data.end_index;
            response_data *td_arr=(response_data*)malloc(sizeof(response_data)*(end-start+1));
            for(int k=start;k<=end;k++){
                int m=k-start;
                td_arr[m]=td.response_ref_ptr[k];
                printf("thread=%d,Status_code=>%d\n",td.thread_id,td.response_ref_ptr[k].Status_code);
                // printf("Response_header=%s\n",td.response_ref_ptr[k].Resp_header);
                printf("Response_body1=>%s\n",td_arr[m].Response_body);
            }
            // printf("len=%d\n",total_requests);
            char * serialized=thread_data_to_json(*td_arr,end-start+1,start,end);
            // printf("serialized=%s\n",serialized);
            // char bytes[sizeof(td)];
            // memcpy(bytes, &td, sizeof(td));

            // printf("data to send1=%s,len=%ld\n", bytes, sizeof(bytes));
            // for (int l = 0; l < sizeof(bytes); l++)
            //     printf("%02X ", bytes[l]);
            // printf("\n");

            // response_data_to_json(td);
            send_data(serialized);
            exit(0);
        }
    }

    for (int p = 0; p < thread_size; p++)
    {
        wait(NULL);
    }
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
    printf("thread_size=%d\n", thread_size);
    uv_thread_t *threads = (uv_thread_t *)malloc(sizeof(uv_thread_t) * thread_size);
    thread_data *threads_data = (thread_data *)malloc(sizeof(thread_data) * thread_size);

    update_response_data_t th_data;
    th_data.thread_size = thread_size;
    th_data.response_ref = response_ref;

    pthread_t thread;
    pthread_create(&thread, NULL, run_update_response_data, (void *)&th_data);

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return;
    }

    for (int i = 0; i < thread_size; i++)
    {
        threads_data[i].req_inputs_ptr = req_inputs;
        threads_data[i].response_ref_ptr = response_ref;
        threads_data[i].debug_flag = debug;
        threads_data[i].thread_id = i;
        threads_data[i].th_pool_data = proc_data[i];
        threads_data[i].api_req_async_on_thread = new api_req_async(i, &lock, curl_multi_init());
    }

    create_process(thread_size, total_requests,threads, threads_data, proc_data);
    // printf("pid=%d\n", getpid());
    pthread_cancel(thread);
    close(*receive_data_sockfd);

    pthread_join(thread, NULL);

    printf("\n\n--------- end -----------\n\n");
}