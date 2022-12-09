#include "api_req_async.hpp"

// #define DEFAULT_PORT 0
#define DEFAULT_BACKLOG 128

uv_loop_t *loop;
struct sockaddr_in addr;

typedef struct
{
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

void free_write_req(uv_write_t *req)
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
    free_write_req(req);
}

void my_tcp_server::write2client(uv_stream_t *stream, char *data, size_t len2)
{
    uv_buf_t buffer[] = {
        {.base = data, .len = len2}};
    printf("write2client=%s\n", data);
    uv_write_t *req = (uv_write_t *)malloc(sizeof(uv_write_t));
    uv_write(req, stream, buffer, 1, echo_write);
}

void my_tcp_server::echo_read(uv_stream_t *client_stream, ssize_t nread, const uv_buf_t *buf)
{
    if (nread > 0)
    {
        write_req_t *req = (write_req_t *)malloc(sizeof(write_req_t));
        req->buf = uv_buf_init(buf->base, nread);
        printf("\necho_read:\t  %s\n", (char *)(req->buf.base));
        StringType *raw_response;
        raw_response->ch = req->buf.base;
        raw_response->length = req->buf.len;
        get_received_data_cb(raw_response, client_stream);
        free(raw_response);
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

void my_tcp_server::on_new_connection(uv_stream_t *server, int status)
{
    printf("on_new_connection\n");
    if (status < 0)
    {
        printf("New connection error %s\n", uv_strerror(status));
        // error!
        return;
    }
    printf("connection accepted...\n");

    auto echo_read_with_context = [=](uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) -> void
    {
        echo_read(client, nread, buf);
    };
    auto _closure_echo_read = Closure_echo_read::create<void>(echo_read_with_context);

    uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);
    if (uv_accept(server, (uv_stream_t *)client) == 0)
    {
        uv_read_start((uv_stream_t *)client, alloc_buffer, _closure_echo_read);
    }
    else
    {
        uv_close((uv_handle_t *)client, NULL);
    }
}

struct Closure_on_new_connection
{
    template <typename Any, typename RETURN_TYPE>
    static Any lambda_ptr_exec(uv_stream_t *server, int status)
    {
        return (Any)(*(RETURN_TYPE *)callback<RETURN_TYPE>())(server, status);
    }

    template <typename Any = void, typename CALLER_TYPE = Any (*)(uv_stream_t *server, int status), typename RETURN_TYPE>
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

int my_tcp_server::start_server()
{
    server_ready=0;
    loop = uv_default_loop();
    uv_tcp_init(loop, &server);
    uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &addr);
    int r;
    r = uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);
    if (r)
    {
        printf("Bind error %d\n", r);
        uv_stop(loop);
        return -1;
    }

    auto on_new_connection_with_context = [=](uv_stream_t *server, int status) -> void
    {
        on_new_connection(server, status);
    };
    auto _closure_on_new_connection = Closure_on_new_connection::create<void>(on_new_connection_with_context);

    r = uv_listen((uv_stream_t *)&server, DEFAULT_BACKLOG, _closure_on_new_connection);
    if (r)
    {
        printf("Listen error %s\n", uv_strerror(r));
        uv_stop(loop);
        return -1;
    }

    {
        int len = sizeof(struct sockaddr_in);
        if (uv_tcp_getsockname(&server, (struct sockaddr *)&addr, &len) == -1)
        {
            perror("getsockname");
            exit(1);
        }
        printf("Socket successfully binded  on port=%d..\n", htons(addr.sin_port));
    }
    server_ready=1;
    return uv_run(loop, UV_RUN_DEFAULT);
}

void my_tcp_server::register_ipc_received_callback(ipc_received_cb_data_type get_received_data_cb)
{
    this->get_received_data_cb = get_received_data_cb;
}

void my_tcp_server::stop_server()
{
    uv_stop(loop);
}

my_tcp_server::my_tcp_server(int port)
{
    this->DEFAULT_PORT = port;
}
