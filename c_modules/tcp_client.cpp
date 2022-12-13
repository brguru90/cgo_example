#include "api_req_async.hpp"

// #define DEFAULT_PORT 0
#define DEFAULT_BACKLOG 128

typedef struct
{
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

void my_tcp_client::free_write_req(uv_write_t *req)
{
    write_req_t *wr = (write_req_t *)req;
    free(wr->buf.base);
    free(wr);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = (char *)malloc(suggested_size);
    buf->len = suggested_size;
}

void echo_write(uv_write_t *req, int status)
{
    if (status)
    {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    // free_write_req(req);
}

uv_write_t *my_tcp_client::write2server(uv_stream_t *stream, char *data, size_t len2, uv_write_t *req)
{
    uv_buf_t buffer[] = {
        {.base = data, .len = len2}};
    // printf("write2server=%s\n", data);
    if (req == nullptr)
    {
        req = (uv_write_t *)malloc(sizeof(uv_write_t));
    }
    uv_write(req, stream, buffer, 1, echo_write);
    return req;
}

void my_tcp_client::echo_read(uv_stream_t *client_stream, ssize_t nread, const uv_buf_t *buf,ipc_received_cb_data_type *cb )
{
    if (nread > 0)
    {
        write_req_t *req = (write_req_t *)malloc(sizeof(write_req_t));
        req->buf = uv_buf_init(buf->base, nread);
        // printf("\necho_read:\t  %s\n", (char *)(req->buf.base));

        StringType raw_response;
        raw_response.ch=req->buf.base;
        raw_response.length=req->buf.len;
        if(cb!=nullptr){
            (*cb)(&raw_response,nullptr);
        }
        // uv_write((uv_write_t *)req, client_stream, &req->buf, 1, echo_write);
        return;
    }
    if (nread < 0)
    {
        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t *)client_stream, NULL);
    }

    free(buf->base);
}

struct Closure_echo_read
{
    template <typename Any, typename RETURN_TYPE>
    static Any lambda_ptr_exec(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
    {
        return (Any)(*(RETURN_TYPE *)callback<RETURN_TYPE>())(client, nread, buf);
    }

    template <typename Any = void, typename CALLER_TYPE = Any (*)(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf), typename RETURN_TYPE>
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

void my_tcp_client::on_connect(uv_connect_t *client, int status)
{
    printf("on_connect\n");
    if (status < 0)
    {
        printf("New client connection error %s,port=%d\n", uv_strerror(status), DEFAULT_PORT);
        // error!
        return;
    }
    printf("connected...\n");

    uv_stream_t *stream = client->handle;
    // free(client);
    StringType *raw_response;
    get_received_data_cb(raw_response, stream);
    free(raw_response);
}

void my_tcp_client::read_response(uv_stream_t *stream, ipc_received_cb_data_type *cb)
{
    auto echo_read_with_context = [=](uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) -> void
    {
        echo_read(client, nread, buf, cb);
    };
    auto _closure_echo_read = Closure_echo_read::create<void>(echo_read_with_context);
    uv_read_start(stream, alloc_buffer, _closure_echo_read);
}

struct Closure_on_connect
{
    template <typename Any, typename RETURN_TYPE>
    static Any lambda_ptr_exec(uv_connect_t *client, int status)
    {
        return (Any)(*(RETURN_TYPE *)callback<RETURN_TYPE>())(client, status);
    }

    template <typename Any = void, typename CALLER_TYPE = Any (*)(uv_connect_t *client, int status), typename RETURN_TYPE>
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

int my_tcp_client::start_client()
{
    uv_tcp_init(loop, &client);
    uv_ip4_addr("127.0.0.1", DEFAULT_PORT, &addr);
    int r;
    auto on_connect_with_context = [=](uv_connect_t *client, int status) -> void
    {
        on_connect(client, status);
    };
    auto _closure_on_connect = Closure_on_connect::create<void>(on_connect_with_context);
    uv_connect_t *pConn = (uv_connect_t *)malloc(sizeof(uv_connect_t));
    uv_tcp_connect(pConn, &client, (const struct sockaddr *)&addr, _closure_on_connect);
    return uv_run(loop, UV_RUN_DEFAULT);
}

void my_tcp_client::register_ipc_received_callback(ipc_received_cb_data_type *get_received_data_cb)
{
    this->get_received_data_cb = *get_received_data_cb;
}

void my_tcp_client::stop_client()
{
    uv_stop(loop);
}

my_tcp_client::my_tcp_client(int port)
{
    this->DEFAULT_PORT = port;
    loop = uv_loop_new();
}