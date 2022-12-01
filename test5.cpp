#include <stdio.h>
#include <string>
#include <string.h>
#include <cstdlib>
#include <sys/mman.h>
#include <uv.h>
#include <inttypes.h>
#include <unistd.h>

using namespace std;

// struct child_worker
// {
//     uv_process_t req;
//     uv_process_options_t options;
//     uv_pipe_t pipe;
// } * workers;

// void on_exit(uv_process_t *req, int64_t exit_status, int term_signal)
// {
//     fprintf(stderr, "Process exited with status %" PRId64 ", signal %d\n", exit_status, term_signal);
//     uv_close((uv_handle_t *)req, NULL);
// }

// int setup_process()
// {
//     uv_loop_t *loop;
//     uv_process_t child_req;
//     uv_process_options_t options;
//     loop = uv_default_loop();

//     // char* args[3];
//     // args[0] = (char*)"mkdir";
//     // args[1] = (char*)"test-dir";
//     // args[2] = NULL;

//     options.exit_cb = on_exit;
//     // options.file = "mkdir";
//     // options.args = args;

//     int r;
//     if ((r = uv_spawn(loop, &child_req, &options)))
//     {
//         printf("%s\n", uv_strerror(r));
//         return 1;
//     }
//     else
//     {
//         printf( "Launched process with ID %d\n", child_req.pid);
//     }
//     printf("--------------\n");
//     return uv_run(loop, UV_RUN_DEFAULT);
//     printf("--------------,,\n");
// }

// int main()
// {
//     setup_process();
//     return 0;
// }

typedef struct child2_type
{
    char *name = (char *)malloc(100);
    int *a;
} child2_t;

typedef struct child1_type
{
    child2_t *child2;
} child1_t;

typedef struct parent_type
{
    child1_t *child1;
    int *k;
} parent_t;

int main()
{
    char s[] = "abc";
    int m = 4;
    string name = "guru";
    child2_t a;
    a.name = s;
    a.a = &m;

    child1_t b;
    b.child2 = &a;

    parent_t c;
    c.k = &m;
    c.child1 = &b;

    parent_t *c2 = &c;

    parent_t *c3 = (parent_t *)(void *)c2;
    // char mmm[]="guru";

    unsigned char bytes[sizeof(c)];
    if (!fork())
    {
        memcpy(bytes, &c, sizeof(c));
        exit(0);
        // bytes[sizeof(*c3)]='\0';
    }

    char *my_s_bytes = reinterpret_cast<char *>(c3);

    printf("raw bytes=[[%s\n]][[%s]]\n", bytes, my_s_bytes);
    for (int i = 0; i < sizeof(bytes); i++)
        printf("%02X ", bytes[i]);
    printf("\n");
    parent_t tmp;
    memcpy(&tmp, bytes, sizeof(tmp));

    printf("\n%d\n", *tmp.child1->child2->a);
    printf("%s\n", tmp.child1->child2->name);

    return 0;
}