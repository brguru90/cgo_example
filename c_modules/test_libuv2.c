
#include <uv.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>

void on_exit2(uv_process_t *req, int64_t exit_status, int term_signal)
{
    fprintf(stderr, "Process exited with status %" PRId64 ", signal %d\n", exit_status, term_signal);
    uv_close((uv_handle_t *)req, NULL);
}

#include <uv.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define FIB_UNTIL 1000

uv_work_t fib_reqs[FIB_UNTIL];

int fib_(int n)
{
    if (n <= 1)
        return n;
    return fib_(n - 1) + fib_(n - 2);
}

void fib(uv_work_t *req)
{
    int n = *(int *)req->data;
    if (random() % 2)
        sleep(1);
    else
        sleep(3);
    long fib = fib_(n);
    fprintf(stderr, "%dth fibonacci is %lu\n", n, fib);
}

void after_fib(uv_work_t *req, int status)
{
    if (status == UV_ECANCELED)
        fprintf(stderr, "Calculation of %d cancelled.\n", *(int *)req->data);
}

void signal_handler(uv_signal_t *req, int signum)
{
    printf("Signal received!\n");
    int i;
    for (i = 0; i < FIB_UNTIL; i++)
    {
        uv_cancel((uv_req_t *)&fib_reqs[i]);
    }
    uv_signal_stop(req);
}

int run_process()
{
    uv_loop_t *loop = uv_loop_new();
    int i;
    int data[FIB_UNTIL];
    for (i = 0; i < FIB_UNTIL; i++)
    {
        data[i] = i;
        fib_reqs[i].data = (void *)&data[i];
        uv_queue_work(loop, &fib_reqs[i], fib, after_fib);
    }

    uv_signal_t sig;
    uv_signal_init(loop, &sig);
    uv_signal_start(&sig, signal_handler, SIGINT);

    return uv_run(loop, UV_RUN_DEFAULT);
}

uv_loop_t *main_loop;
uv_process_t child_req;
uv_process_options_t options;

// int main() {
//     main_loop = uv_default_loop();

//     char* args[3];
//     args[0] = "mkdir";
//     args[1] = "test-dir";
//     args[2] = NULL;

//     options.exit_cb = on_exit2;
//     options.file = "mkdir";
//     options.args = args;

//     int r;
//     if ((r = uv_spawn(main_loop, &child_req, &options))) {
//         fprintf(stderr, "%s\n", uv_strerror(r));
//         return 1;
//     } else {
//         fprintf(stderr, "Launched process with ID %d\n", child_req.pid);
//         run_process();
//     }

//     return uv_run(main_loop, UV_RUN_DEFAULT);
// }

struct child_worker
{
    uv_process_t req;
    uv_process_options_t options;
    uv_pipe_t pipe;
} * workers;

int main()
{
    main_loop = uv_default_loop();
    int cpu_count = 5;

    struct child_worker *workers = calloc(cpu_count, sizeof(struct child_worker));
    while (cpu_count--)
    {
        struct child_worker *worker = &workers[cpu_count];
        uv_pipe_init(main_loop, &worker->pipe, 1);

        char *args[3];
        args[0] = "mkdir";
        args[1] = "test-dir";
        args[2] = NULL;

        uv_stdio_container_t child_stdio[3];
        child_stdio[0].flags = UV_CREATE_PIPE | UV_READABLE_PIPE;
        child_stdio[0].data.stream = (uv_stream_t *)&worker->pipe;
        child_stdio[1].flags = UV_IGNORE;
        child_stdio[2].flags = UV_INHERIT_FD;
        child_stdio[2].data.fd = 2;

        worker->options.stdio = child_stdio;
        worker->options.stdio_count = 3;

        worker->options.exit_cb = on_exit2;
        worker->options.file = "mkdir";
        worker->options.args = args;

        int r;
        if ((r = uv_spawn(main_loop, &worker->req, &worker->options)))
        {
            fprintf(stderr, "%s\n", uv_strerror(r));
            return 1;
        }
        else
        {
            fprintf(stderr, "Launched process with ID %d\n", child_req.pid);
            run_process();
        }
        fprintf(stderr, "Started worker %d\n", worker->req.pid);
    }
    return uv_run(main_loop, UV_RUN_DEFAULT);
}



// gcc  '/home/justdial/Desktop/test_workspace/test_libuv2.c' -luv && ./a.out