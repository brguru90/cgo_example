#include <stdio.h>
#include "api_req.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_THREAD 5


void *goCallback_wrap(void *vargp){
    int *myid = (int *)vargp;
    goCallback(*myid);
    pthread_exit(NULL);
}

void run_bulk_api_request(char *s){
    printf("%s\n",s);

    int i;
    pthread_t threads[NUM_THREAD];
    pthread_t tid;

    for (i = 0; i < NUM_THREAD; i++){
        pthread_create(&threads[i], NULL, goCallback_wrap, (void *)&threads[i]);
    }

    for (i = 0; i < NUM_THREAD; i++){
        pthread_join(threads[i], NULL);
    }
 
    // printf("Main thread exiting...\n");
    // pthread_exit(NULL);

    goCallback(-1);
}