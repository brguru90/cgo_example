#include "api_req.h"

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

// #define BUFFER_SIZE 1024

// sock_type socket_connect(char *host, in_port_t port, bool secure, bool debug)
// {
//     struct hostent *hp;
//     struct sockaddr_in addr;
//     int on = 1;
//     sock_type sock;

//     if ((hp = gethostbyname(host)) == NULL)
//     {
//         herror("gethostbyname");
//         exit(1);
//     }
//     // copy(hp->h_addr, &addr.sin_addr, hp->h_length);

//     memset(&addr, 0, sizeof(addr));
//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(port);
//     memcpy(&addr.sin_addr.s_addr, hp->h_addr_list[0], hp->h_length);

//     // addr.sin_port = htons(port);
//     // addr.sin_family = AF_INET;
//     sock.socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
//     setsockopt(sock.socket, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));

//     if (sock.socket == -1)
//     {
//         perror("setsockopt");
//         exit(1);
//     }

//     if (debug)
//     {
//         printf("connecting %s:%d,secure=%d\n", host, port, secure);
//     }

//     if (connect(sock.socket, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
//     {
//         perror("connect");
//         exit(1);
//     }
//     if (secure)
//     {

//         if (debug)
//         {
//             printf("establishing ssl\n");
//         }

//         SSL_load_error_strings();
//         SSL_library_init();
//         SSL_CTX *ssl_ctx = SSL_CTX_new(SSLv23_client_method());
//         sock.secureConn = SSL_new(ssl_ctx);
//         if (SSL_set_fd(sock.secureConn, sock.socket) == -1)
//         {
//             // perror("error in establishing secure connection");
//             printf("error in establishing secure connection");
//             exit(1);
//         }
//     }
//     return sock;
// }

// response_data send_raw_request(char *host, in_port_t port, bool secure, char *raw_req, int debug)
// {
//     response_data rd;
//     char *raw_response;
//     char buffer[BUFFER_SIZE];
//     char small_buffer[2];

//     // printf("%s:%d,%d\n%s\n\n", host, port, secure, raw_req);

//     rd.before_connect_time = time(0);
//     sock_type sock = socket_connect(host, port, secure, debug > 0);
//     rd.connect_time = time(0);
//     // if (debug > 1)
//     // {
//     //     printf("%s\n\n", raw_req);
//     // }
//     write(sock.socket, raw_req, strlen(raw_req)); // write request & send

//     // read response
//     bool read_single_char = false;
//     bzero(buffer, BUFFER_SIZE);
//     int cond=0;
//     // while ((cond=read(sock.socket, read_single_char ? buffer : small_buffer,
//     //             read_single_char ? BUFFER_SIZE - 1 : 2)) != 0)
//     // {
//     //     printf("--%d,%d\n",read_single_char,cond);
//     //     if (read_single_char)
//     //     {
//     //         fprintf(stderr, "1, %s", buffer);
//     //         // strcpy(raw_response, buffer);
//     //         bzero(buffer, BUFFER_SIZE);
//     //     }
//     //     else
//     //     {
//     //         fprintf(stderr, "2, %s", small_buffer);
//     //         // strcpy(raw_response, buffer);
//     //         bzero(small_buffer, 2);
//     //         read_single_char = true;
//     //         rd.time_at_first_byte = time(0);
//     //     }
//     //     printf(".,\n");
//     // }

//     while((cond=read(sock.socket, buffer, BUFFER_SIZE - 1))>0){
//         printf("%d)\n",cond);
// 		fprintf(stderr, "%s", buffer);
// 		bzero(buffer, BUFFER_SIZE);
//         printf("..,\n");
// 	}
//     if(debug>0){
//         printf("request finished");
//     }
//     rd.response = raw_response;
//     // if (debug > 2)
//     // {
//     //     printf("%s\n\n", raw_response);
//     // }

//     if (secure)
//     {
//         SSL_shutdown(sock.secureConn);
//     }
//     shutdown(sock.socket, SHUT_RDWR);
//     close(sock.socket);

//     return rd;
// }

static int wait_on_socket(curl_socket_t sockfd, int for_recv, long timeout_ms)
{
  struct timeval tv;
  fd_set infd, outfd, errfd;
  int res;
 
  tv.tv_sec = timeout_ms / 1000;
  tv.tv_usec = (timeout_ms % 1000) * 1000;
 
  FD_ZERO(&infd);
  FD_ZERO(&outfd);
  FD_ZERO(&errfd);
 
  FD_SET(sockfd, &errfd); /* always check for error */
 
  if(for_recv) {
    FD_SET(sockfd, &infd);
  }
  else {
    FD_SET(sockfd, &outfd);
  }
 
  /* select() returns the number of signalled sockets or -1 */
  res = select((int)sockfd + 1, &infd, &outfd, &errfd, &tv);
  return res;
}

response_data send_raw_request(char *host, in_port_t port, bool secure, char *raw_req, int debug)
{
    printf("%s",host);
    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl)
    {
        curl_socket_t sockfd;
        curl_easy_setopt(curl, CURLOPT_URL, "http://postit.example.com/moo.cgi");
        curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);
        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res == CURLE_OK)
        {
            size_t nsent = 0;
            res = curl_easy_getinfo(curl, CURLINFO_ACTIVESOCKET, &sockfd);
            res = curl_easy_send(curl, raw_req, strlen(raw_req), &nsent);
            if (res != CURLE_OK)
            {
                printf("Error: %s\n", curl_easy_strerror(res));
                exit(1);
            }

            for (;;)
            {
                /* Warning: This example program may loop indefinitely (see above). */
                char buf[1024];
                size_t nread;
                do
                {
                    nread = 0;
                    res = curl_easy_recv(curl, buf, sizeof(buf), &nread);

                    if (res == CURLE_AGAIN && !wait_on_socket(sockfd, 1, 60000L))
                    {
                        printf("Error: timeout.\n");
                        exit(1);
                    }
                } while (res == CURLE_AGAIN);

                if (res != CURLE_OK)
                {
                    printf("Error: %s\n", curl_easy_strerror(res));
                    break;
                }

                if (nread == 0)
                {
                    /* end of the response */
                    break;
                }

                printf("Received %lu bytes.\n", (unsigned long)nread);
            }
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}