#include "api_req_async.hpp"

void *loop_on_the_thread(void *data)
{
    thread_data *td = (thread_data *)data;
    td->api_req_async_on_thread.run(td);
    return NULL;
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
    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_size);
    thread_data *threads_data = (thread_data *)malloc(sizeof(thread_data) * thread_size);
    // api_req_async *api_req_async_on_thread=(api_req_async*)malloc(sizeof(api_req_async)*thread_size);

    for (int i = 0; i < thread_size; i++)
    {
        threads_data[i].req_inputs_ptr = req_inputs;
        threads_data[i].response_ref_ptr = response_ref;
        threads_data[i].debug_flag = debug;
        threads_data[i].thread_id = i;
        threads_data[i].th_pool_data = proc_data[i];
        threads_data[i].api_req_async_on_thread = api_req_async();
    }

    for (int p = 0; p < thread_size; p++)
    {
        if (pthread_create(&threads[p], NULL, loop_on_the_thread, (void *)&threads_data[p]) != 0)
        {
            perror("pthread_create() error");
            exit(1);
        }
    }

    for (int i = 0; i < thread_size; i++)
    {
        pthread_join(threads[i], NULL);
    }
    free(threads);
    free(threads_data);
}