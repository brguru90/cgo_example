#include "api_req.h"

// https://gist.github.com/nolim1t/126991/ae3a7d36470d2a81190339fbc78821076a4059f7
// https://github.com/ppelleti/https-example/blob/master/https-client.c
// https://stackoverflow.com/questions/40303354/how-to-make-an-https-request-in-c
// https://stackoverflow.com/questions/22077802/simple-c-example-of-doing-an-http-post-and-consuming-the-response
// https://stackoverflow.com/questions/62387069/golang-parse-raw-http-2-response

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

#define BUFFER_SIZE 1024

sock_type socket_connect(char *host, in_port_t port, bool secure, bool debug)
{
    struct hostent *hp;
    struct sockaddr_in addr;
    int on = 1;
    sock_type sock;

    if ((hp = gethostbyname(host)) == NULL)
    {
        herror("gethostbyname");
        exit(1);
    }
    // copy(hp->h_addr, &addr.sin_addr, hp->h_length);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr.s_addr, hp->h_addr_list[0], hp->h_length);

    // addr.sin_port = htons(port);
    // addr.sin_family = AF_INET;
    sock.socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    setsockopt(sock.socket, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));

    if (sock.socket == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    if (debug)
    {
        printf("connecting %s:%d,secure=%d\n", host, port, secure);
    }

    if (connect(sock.socket, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
    {
        perror("connect");
        exit(1);
    }
    if (secure)
    {

        if (debug)
        {
            printf("establishing ssl\n");
        }

        SSL_load_error_strings();
        SSL_library_init();
        SSL_CTX *ssl_ctx = SSL_CTX_new(SSLv23_client_method());
        sock.secureConn = SSL_new(ssl_ctx);
        if (SSL_set_fd(sock.secureConn, sock.socket) == -1)
        {
            // perror("error in establishing secure connection");
            printf("error in establishing secure connection");
            exit(1);
        }
    }
    return sock;
}

long long get_current_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

long get_max(long a, long b)
{
    return a > b ? a : b;
}

char *str_slice(char *src, int start, int end)
{
    char *temp = malloc(get_max(end, start + 1) - start);
    bzero(temp, BUFFER_SIZE);
    for (int i = start; i < end && src[i] != '\0'; i++)
    {
        strncat(temp, &src[i], 1);
    }
    return temp;
}

typedef struct ContentLengthInfo
{
    long content_length;
    int position_index;
} content_length_info;

content_length_info extract_content_length(char *s)
{
    content_length_info cn;
    cn.content_length = -1;
    int i = 0;
    int _len = strlen(s);
    char *line = malloc(1024);
    while (i < _len)
    {
        bzero(line, BUFFER_SIZE);
        int lint_start = i;
        while (i < _len && s[i] != '\n')
        {
            // printf("%c\n", (char)s[i]);
            // strcat(line, "(char)s[i]");
            strncat(line, &s[i], 1);
            i++;
        }
        // printf("-------line %s----\n",line);
        // printf("%d/%d,%d\n",i,_len,s[i]=='\n');
        if (strstr(line, "Content-Length: "))
        {
            free(line);
            printf("-------match----\n");
            char *c_len_str = malloc(1024);
            for (int j = i; j >= lint_start; j--)
            {
                // printf("%c",s[j]);
                if (s[j] == ' ' && (s[j - 1] == ':' || s[j - 1] == ' '))
                {
                    break;
                }
                if (isdigit(s[j]))
                {
                    strncat(c_len_str, &s[j], 1);
                }
            }
            // return [atoi(c_len_str)];
            cn.content_length = atoi(c_len_str);
            cn.position_index = i;
            return cn;
        }
        i++;
    }
    free(line);
    return cn;
}

response_data send_raw_request(char *host, in_port_t port, bool secure, char *raw_req, int debug)
{
    response_data rd;
    char *raw_response = malloc(1);
    char buffer[BUFFER_SIZE];
    char small_buffer[1];

    // printf("%s:%d,%d\n%s\n\n", host, port, secure, raw_req);

    rd.before_connect_time = time(0);
    sock_type sock = socket_connect(host, port, secure, debug > 0);
    rd.connect_time = time(0);
    // if (debug > 1)
    // {
    //     printf("%s\n\n", raw_req);
    // }
    // write(sock.socket, raw_req, strlen(raw_req)); // write request & send
    send(sock.socket, raw_req, strlen(raw_req), 0);

    // read response
    bool read_single_char = false;
    bzero(buffer, BUFFER_SIZE);
    int cond = 0, total_body_received = 0;
    content_length_info cn;
    cn.content_length = -1;
    int time_out_millisec = 2000;
    long long last_response_at = get_current_time();

    // https://linuxhint.com/c-recv-function-usage/
    fcntl(sock.socket, F_SETFL, O_NONBLOCK);
    while ((get_current_time() - last_response_at) <= time_out_millisec)
    {
        // printf(".,\n");
        // printf("~~%lld\n", (get_current_time() - last_response_at));
        if ((cond = recv(sock.socket, read_single_char ? buffer : small_buffer,
                         read_single_char ? BUFFER_SIZE - 1 : 1, 0)) < 0)
        {
            sleep(0.01);
            continue;
        }
        // fprintf(stderr, "%s", buffer);
        strcpy(raw_response, buffer);
        if (!read_single_char)
        {
            read_single_char = true;
            rd.time_at_first_byte = time(0);
        }
        last_response_at = get_current_time();

        if (cn.content_length == -1)
        {
            if ((cn = extract_content_length(raw_response)).content_length != -1)
            {
                // printf("\nCN:=%ld,pos=%d\n", cn.content_length,cn.position_index);
                // printf("body=%s;;",str_slice(raw_response,cn.position_index+3,strlen(raw_response)));
                total_body_received = strlen(raw_response) - (cn.position_index + 3);
            }
        } else{
            total_body_received+=strlen(buffer);
            if(total_body_received>=cn.content_length){
                break;
            }
        }
        bzero(buffer, BUFFER_SIZE);
    }

    printf("bytes recived=%d\n", total_body_received);

    // while ((cond = recv(sock.socket, read_single_char ? buffer : small_buffer,
    //                     read_single_char ? BUFFER_SIZE - 1 : 1, 0)) != 0)
    // {
    //     // printf("--%d,%d\n", read_single_char, cond);
    //     if (read_single_char)
    //     {
    //         fprintf(stderr, "%s", buffer);
    //         strcpy(raw_response, buffer);
    //         bzero(buffer, BUFFER_SIZE);
    //     }
    //     else
    //     {
    //         fprintf(stderr, " %s", small_buffer);
    //         strcpy(raw_response, buffer);
    //         bzero(small_buffer, 2);
    //         read_single_char = true;
    //         rd.time_at_first_byte = time(0);
    //     }
    //     printf("\nCN:=%ld\n", extract_content_length(raw_response));
    //     printf(".,\n");
    // }
    // fcntl(sock.socket,F_SETFL,O_NONBLOCK);
    // printf("time=%lld\n", get_current_time());

    // while ((cond = recv(sock.socket, buffer, BUFFER_SIZE - 1, 0)) > 0)
    // {
    //     printf("%d)\n", cond);
    //     fprintf(stderr, "%s", buffer);
    //     bzero(buffer, BUFFER_SIZE);
    //     printf("..,\n");
    // }
    if (debug > 0)
    {
        printf("request finished");
    }
    rd.response = raw_response;
    if (debug > 2)
    {
        printf("%s\n\n", raw_response);
    }

    if (secure)
    {
        SSL_shutdown(sock.secureConn);
    }
    shutdown(sock.socket, SHUT_RDWR);
    close(sock.socket);
    printf("\nshutdown\n");

    return rd;
}
