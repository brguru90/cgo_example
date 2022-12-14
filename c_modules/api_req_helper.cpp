#include "api_req_async.hpp"

using namespace std;

#define BUFFER_SIZE 100
// #define PORT 14502
#define PORT_MIN 49152
#define PORT_MAX 65535
#define SA struct sockaddr

int PORT = (rand() % (PORT_MAX + 1 - PORT_MIN)) + PORT_MIN;

pthread_mutex_t lock;

char end_of_data[] = "end_of_data";

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

// void my_strcpy(StringType &dest, char *src, int length)
// {
//     int prev_length = dest.length;
//     char temp[prev_length];
//     // resize & repopulate
//     memcpy(&temp, dest.ch, prev_length);
//     dest.ch = (char *)malloc(sizeof(char *) * (prev_length + length + 1));
//     memcpy(dest.ch, &temp, prev_length);
//     int j = prev_length;
//     for (int i = 0; i < length; i++)
//     {
//         // printf("copy %02X ", dest.ch[prev_length + i]);
//         dest.ch[j + i] = src[i];
//     }
//     dest.length = prev_length + length;
// }

// int isSubString(StringType &dest, char end_of_data[])
// {
//     int i = dest.length;
//     // printf("i->%d,%d\n",i,i>=0);
//     int end_len = strlen(end_of_data);
//     while (i-- >= 0)
//     {
//         // printf("i->%d,%d\n",i,i>=0);
//         if (dest.ch[i] == end_of_data[end_len - 1])
//         {
//             for (int j = end_len - 1; j >= 0 && i >= 0; j--)
//             {
//                 if (dest.ch[i--] == end_of_data[j])
//                 {
//                     if (j == 0)
//                         return i + end_len + 1;
//                 }
//                 else
//                 {
//                     break;
//                 }
//             }
//         }
//     }
//     return -1;
// }

int *receive_data_sockfd;
bool ipc_server_ready = false;
struct sockaddr_in servaddr, cli;
typedef void (*get_received_data_type)(StringType *raw_response);

struct Closure_my_tcp_server_cb
{
    template <typename Any, typename RETURN_TYPE>
    static Any lambda_ptr_exec(StringType *raw_response, uv_stream_t *client_stream)
    {
        return (Any)(*(RETURN_TYPE *)callback<RETURN_TYPE>())(raw_response, client_stream);
    }

    template <typename Any = void, typename CALLER_TYPE = Any (*)(StringType *raw_response, uv_stream_t *client_stream), typename RETURN_TYPE>
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
my_tcp_server server = my_tcp_server(0);
void receive_data(int thread_size, get_received_data_type get_received_data_cb)
{
    auto cb = [&](StringType *raw_response, uv_stream_t *client_stream) -> void
    {
        // printf("------------get_received_data_cb-----------------\n");
        // ofstream MyFile("./json_bytes_c_recv.json");
        // MyFile << raw_response->ch;
        // MyFile.close();
        get_received_data_cb(raw_response);
        server.write2client(client_stream, end_of_data, strlen(end_of_data));
    };
    auto _closure_cb = Closure_my_tcp_server_cb::create<void>(cb);
    // printf("_closure_cb=%p\n", _closure_cb);
    server.register_ipc_received_callback(&_closure_cb);
    while (server.start_server() == -1)
    {
        usleep(200);
    }
    // servaddr.sin_port=htons(server.addr.sin_port);
    // int sockfd, connfd, len;
    // receive_data_sockfd = &sockfd;

    // printf("server on port=%d\n", PORT);

    // // socket create and verification
    // sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // if (sockfd == -1)
    // {
    //     printf("socket creation failed...\n");
    //     exit(0);
    // }
    // else
    //     printf("Socket successfully created..\n");
    // bzero(&servaddr, sizeof(servaddr));

    // // assign IP, PORT
    // servaddr.sin_family = AF_INET;
    // servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // servaddr.sin_port = 0;

    // // Binding newly created socket to given IP and verification
    // while ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) < 0)
    // {
    //     if (errno == EADDRINUSE)
    //     {
    //         printf("the port is not available. already to other process\n");
    //         exit(1);
    //     }
    //     else
    //     {
    //         printf("could not bind to process (%d) %s\n", errno, strerror(errno));
    //         exit(1);
    //     }
    // }
    // {
    //     socklen_t len = sizeof(servaddr);
    //     if (getsockname(sockfd, (struct sockaddr *)&servaddr, &len) == -1)
    //     {
    //         perror("getsockname");
    //         exit(1);
    //     }
    //     printf("Socket successfully binded  on port=%d..\n", servaddr.sin_port);
    // }

    // // Now server is ready to listen and verification
    // if ((listen(sockfd, 5)) != 0)
    // {
    //     printf("Listen failed...\n");
    //     exit(0);
    // }
    // else
    //     printf("Server listening..\n");
    // len = sizeof(cli);

    // ipc_server_ready=true;
    // int i = thread_size;
    // while (1)
    // {
    //     // Accept the data packet from client and verification
    //     connfd = accept(sockfd, (SA *)&cli, (socklen_t *)&len);
    //     if (connfd < 0)
    //     {
    //         printf("server accept failed...\n");
    //         exit(0);
    //     }
    //     else
    //         printf("server accept the client...\n");

    //     // Function for chatting between client and server
    //     char buffer[BUFFER_SIZE];
    //     bzero(buffer, BUFFER_SIZE);
    //     int n, cond;

    //     // char *raw_response = (char *)malloc(1);
    //     StringType raw_response;
    //     // raw_response.ch=(char*)malloc(1);
    //     raw_response.length = 0;
    //     raw_response.ch = (char *)"";
    //     while ((cond = recv(connfd, buffer, BUFFER_SIZE - 1, 0)) > 0)
    //     {
    //         // printf("%d)\n", cond);
    //         // printf("Received:\n");
    //         // for (int l = 0; l < sizeof(buffer); l++)
    //         //     printf("%02X ", buffer[l]);
    //         // printf("\n");
    //         // strcpy(raw_response, buffer);
    //         my_strcpy(raw_response, buffer, sizeof(buffer) - 1);

    //         // strncpy(raw_response, buffer,sizeof(buffer));
    //         // printf("raw_response=%s,size=%d\n", raw_response.ch, raw_response.length);
    //         // if ( strstr(raw_response.ch,end_of_data) != NULL)
    //         int pos = 0;
    //         if ((pos = isSubString(raw_response, end_of_data)) >= 0)
    //         {
    //             // printf("end_of_data,%d\n", pos);
    //             raw_response.length = pos;
    //             get_received_data_cb(&raw_response);
    //             raw_response.ch = (char *)"";
    //             raw_response.length = 0;
    //             // just sending data to sender that wait, so that sub process do not exit
    //             send(connfd, end_of_data, strlen(end_of_data), 0);
    //             break;
    //         }
    //         bzero(buffer, BUFFER_SIZE);
    //     }
    //     close(connfd);
    // }
    // close(sockfd);
}

void send_data(string_type main_raw_response, int start)
{
    // int _size = serialized.length + strlen(end_of_data);
    // char bytes[_size];
    // memcpy(bytes, serialized, strlen(serialized));

    // StringType main_raw_response;
    // main_raw_response.length = 0;
    // my_strcpy(main_raw_response, serialized, strlen(serialized));
    my_strcpy(main_raw_response, end_of_data, strlen(end_of_data));

    // char buffer2[1024];
    // snprintf(buffer2, sizeof(buffer2), "./json_bytes2_%d.json", start);
    // ofstream MyFile(buffer2);
    // MyFile << main_raw_response.ch;
    // MyFile.close();

    // for (int i = strlen(serialized), j = 0; i < _size && j < strlen(end_of_data); i++, j++)
    // {
    //     bytes[i] = end_of_data[j];
    // }

    // printf("data to send2=%s,len=%ld\n", bytes, strlen(bytes));
    // for (int l = 0; l < strlen(bytes); l++)
    //     // printf("%02X ", bytes[l]);
    //      printf("%c", bytes[l]);
    // printf("\n");

    my_tcp_client client = my_tcp_client(htons(server.addr.sin_port));
    auto cb = [&](StringType *raw_response, uv_stream_t *client_stream) -> void
    {
        // char buffer2[1024];
        // snprintf(buffer2, sizeof(buffer2), "./json_bytes_c_send_%d.json", start);
        // ofstream MyFile(buffer2);
        // MyFile << main_raw_response.ch;
        // MyFile.close();
        // uv_write_t *req = client.stream2server(client_stream,main_raw_response,50);
        uv_write_t *req = client.write2server(client_stream, main_raw_response.ch, main_raw_response.length, nullptr);
        auto cb2 = [&](StringType *raw_response2, uv_stream_t *client_stream2)
        {
            // printf("\n\n\n\n\nraw_response2=%s\n",raw_response2->ch);
            // client.free_write_req(req);
            client.stop_client();
        };
        auto _closure_cb2 = Closure_my_tcp_server_cb::create<void>(cb2);
        client.read_response(client_stream, &_closure_cb2);
        // client.free_write_req(req);
        // client.stop_client();
    };
    auto _closure_cb = Closure_my_tcp_server_cb::create<void>(cb);
    client.register_ipc_received_callback(&_closure_cb);
    client.start_client();

    // int sockfd;
    // struct sockaddr_in cli_addr, cli;

    // // socket create and verification
    // sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // if (sockfd == -1)
    // {
    //     printf("socket creation failed...\n");
    //     exit(0);
    // }
    // else
    //     printf("Socket successfully created..\n");
    // bzero(&cli_addr, sizeof(cli_addr));

    // // assign IP, PORT
    // cli_addr.sin_family = AF_INET;
    // cli_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // cli_addr.sin_port = server.addr.sin_port;
    // // cli_addr.sin_family=servaddr.sin_family;
    // // cli_addr.sin_addr=servaddr.sin_addr;

    // // connect the client socket to server socket
    // if (connect(sockfd, (SA *)&cli_addr, sizeof(cli_addr)) != 0)
    // {
    //     printf("connection with the server failed %d,err=%d...\n", cli_addr.sin_port, errno);
    //     exit(0);
    // }
    // else
    //     printf("connected to the server with port=%d..\n", cli_addr.sin_port);

    // // function for chat
    // // write(sockfd, buff, sizeof(buff));
    // send(sockfd, main_raw_response.ch, main_raw_response.length, 0);
    // // send(sockfd, end_of_data, sizeof(end_of_data), 0);
    // // bzero(bytes, strlen(bytes));

    // char buffer[BUFFER_SIZE];
    // bzero(buffer, BUFFER_SIZE);
    // printf("waiting\n");
    // // just make wait sender
    // StringType raw_response;
    // // raw_response.ch=(char*)malloc(1);
    // raw_response.length = 0;
    // raw_response.ch = (char *)"";
    // int cond;
    // bool end_conn_signal = false;
    // while (!end_conn_signal)
    // {
    //     // Accept the replay from server
    //     usleep(200);
    //     while ((cond = recv(sockfd, buffer, BUFFER_SIZE - 1, 0)) > 0)
    //     {
    //         // printf("%d)\n", cond);
    //         // printf("Received:\n");
    //         // for (int l = 0; l < sizeof(buffer); l++)
    //         //     printf("%02X ", buffer[l]);
    //         // printf("\n");
    //         // strcpy(raw_response, buffer);
    //         my_strcpy(raw_response, buffer, sizeof(buffer) - 1);

    //         // strncpy(raw_response, buffer,sizeof(buffer));
    //         printf("raw_response=%s,size=%ld\n", raw_response.ch, raw_response.length);
    //         // if ( strstr(raw_response.ch,end_of_data) != NULL)
    //         int pos = 0;
    //         if ((pos = isSubString(raw_response, end_of_data)) >= 0)
    //         {
    //             printf("wait complete-isSubString\n");
    //             // raw_response.ch = (char *)"";
    //             // raw_response.length = 0;
    //             end_conn_signal = true;
    //             break;
    //         }
    //         bzero(buffer, BUFFER_SIZE);
    //     }
    // }
    // printf("wait complete-%s\n", raw_response.ch);

    // // close the socket
    // close(sockfd);
    // printf("client socket closed successfully\n");
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
        printf("\n\nlen of raw final data from IPC->%ld,%ld\n", raw_response->length, strlen(raw_response->ch));
        raw_response->length = raw_response->length - strlen(end_of_data);
        // printf("\nlast char=%c", raw_response->ch[raw_response->length - 1]);
        // printf("\n\nraw final data from IPC->%ld,%s\n", raw_response->length, raw_response->ch);
        // char tmp[raw_response->length];
        char *tmp = (char *)malloc(sizeof(char) * (raw_response->length + 1));
        bzero(tmp, raw_response->length + 1);
        memcpy(tmp, raw_response->ch, raw_response->length);
        // tmp[raw_response->length]='\0';
        // raw_response->length++;
        // for(int i=raw_response->length-10;i<raw_response->length;i++){
        //     printf("%d=%c\n",i,tmp[i]);
        // }
        // printf("\n\n    final data from IPC->%ld,char_len=%ld,%s\n", raw_response->length,strlen(tmp), tmp);
        // printf("\n");
        // for(int i=raw_response->length-10;i<raw_response->length;i++){
        //     printf("%c-%02X ", tmp[i],tmp[i]);
        // }
        // printf("\n");
        // printf("\n\nlen of final data from IPC->%ld\n", strlen(tmp));
        response_deserialized_type *response_deserialized = json_to_thread_data(tmp, raw_response->length);
        // printf("response_deserialized len=%d\n", response_deserialized->len);
        // // for (int l = 0; l < raw_response->length; l++)
        // //     printf("%02X ", raw_response->ch[l]);
        // // printf("\n");
        for (int i = response_deserialized->start; i <= response_deserialized->end; i++)
        {
            // printf("c res_data=%s\n",response_deserialized->data[i-response_deserialized->start].Response_body);
            response_ref[i] = response_deserialized->data[i - response_deserialized->start];
        }
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

void on_exit2(uv_process_t *req, int64_t exit_status, int term_signal)
{
    fprintf(stderr, "Process exited with status %" PRId64 ", signal %d\n", exit_status, term_signal);
    uv_close((uv_handle_t *)req, NULL);
}

void run_update_response_data(void *data)
{
    update_response_data_type *td = (update_response_data_type *)data;
    update_response_data(td->thread_size, td->response_ref);
}

struct child_worker
{
    uv_process_t req;
    uv_process_options_t options;
    uv_pipe_t pipe;
} *workers;

uv_loop_t *main_loop;
uv_process_t child_req;
uv_process_options_t options;

void create_process(int thread_size, int total_requests, uv_thread_t *threads, thread_data *threads_data, thread_pool_data proc_data[])
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
            response_data *td_arr = (response_data *)malloc(sizeof(response_data) * (end - start + 1));
            for (int k = start; k <= end; k++)
            {
                int m = k - start;
                td_arr[m] = td.response_ref_ptr[k];
                // printf("thread=%d,Status_code=>%d\n",td.thread_id,td.response_ref_ptr[k].Status_code);
                // printf("Response_header=%s\n",td.response_ref_ptr[k].Resp_header);
                // printf("Response_body1=>%d,%d) %s\n",m,k,td_arr[m].Response_body);
            }
            // printf("start=%d,end=%d,len=%d\n",start,end,end-start+1);
            printf("--------- event loop end  -------------\n");
            string_type serialized = thread_data_to_json(td_arr, end - start + 1, start, end);
            threads_data[p].api_req_async_on_thread->~api_req_async();
            printf("--------- serialize end  -------------\n");

            // char buffer[1024];
            // snprintf(buffer, sizeof(buffer),"./json_bytes1_%d.json",start);
            // ofstream MyFile(buffer);
            // MyFile << serialized;
            // MyFile.close();

            // printf("sending serialized with len=%d\n",end-start+1);
            // printf("serialized=%s\n",serialized);
            // char bytes[sizeof(td)];
            // memcpy(bytes, &td, sizeof(td));

            // printf("data to send1=%s,len=%ld\n", bytes, sizeof(bytes));
            // for (int l = 0; l < sizeof(bytes); l++)
            //     printf("%02X ", bytes[l]);
            // printf("\n");
            send_data(serialized, start);
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
    printf("final_process_size=%d\n", thread_size);
    uv_thread_t *threads = (uv_thread_t *)malloc(sizeof(uv_thread_t) * thread_size);
    thread_data *threads_data = (thread_data *)malloc(sizeof(thread_data) * thread_size);

    update_response_data_t th_data;
    th_data.thread_size = thread_size;
    th_data.response_ref = response_ref;

    uv_thread_t hare_id;
    uv_thread_create(&hare_id, run_update_response_data, (void *)&th_data);

    // update_response_data(th_data.thread_size, th_data.response_ref);

    for (int i = 0; i < thread_size; i++)
    {
        threads_data[i].req_inputs_ptr = req_inputs;
        threads_data[i].response_ref_ptr = response_ref;
        threads_data[i].debug_flag = debug;
        threads_data[i].thread_id = i;
        threads_data[i].th_pool_data = proc_data[i];
        threads_data[i].api_req_async_on_thread = new api_req_async(i);
    }

    while (server.server_ready == 0)
    {
        usleep(200);
    }

    create_process(thread_size, total_requests, threads, threads_data, proc_data);
    // close(*receive_data_sockfd);

    printf("\n\n--------- end -----------\n\n");
}

void *ptr_at(void **ptr, int idx)
{
    return ptr[idx];
}