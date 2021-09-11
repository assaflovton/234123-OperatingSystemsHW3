#include "segel.h"
#include "queue.h"
#include "request.h"

//
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// HW3: Parse the new arguments too

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_workers_num = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_full = PTHREAD_COND_INITIALIZER;
void *thread_function(int*args);
int overload_handle(char* sched_alg,int connfd);
int comp (const void * elem1, const void * elem2) ;
int queue_size;
int cur_worker = 0;
time_t seconds;




void getargs(int *port, int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
}

int main(int argc, char *argv[])
{
    //time(&seconds);
    int listenfd, connfd, clientlen;
    struct sockaddr_in clientaddr;
    int *port_p = (int*)malloc(sizeof(int));
    getargs(port_p, argc, argv);
    int port = *port_p;
    int num_of_workers = atoi(argv[2]);//the size of the thread pool
    int queue_max_size = atoi(argv[3]);
    char* sched_alg = (argv[4]); //the algorithm that handel full queue
    pthread_t worker_thread[num_of_workers];
    stats_t* worker_thread_stats=(stats_t*)malloc(sizeof(stats_t)*num_of_workers);

    for (int i = 0; i < num_of_workers; i++) //creating the thread pool
    {
        int* args = (int*)malloc(sizeof(int)*2);
        args[0]=(queue_max_size);
        args[1]=(i);
        pthread_create(&worker_thread[i], NULL, thread_function,(args));
        free(args);
    }

    listenfd = Open_listenfd(port);

    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
        if(queue_size + cur_worker >= queue_max_size){
            if(overload_handle(sched_alg,connfd))
            {
                continue;
            }
        }
        pthread_mutex_lock(&mutex);   //lock
        enqueue((void *)(connfd));
        pthread_mutex_unlock(&mutex); //realse lock
        pthread_cond_signal(&cond);   //send signal that a new request was added to the queue
    }
    free(port_p);
    free(worker_thread_stats);
}

void *thread_function(int* args)
{
    int size=((args[0]));
    int id = ((args[1]));
    stats_t* worker_thread_stats = (stats_t*)malloc(sizeof(stats_t));//collects stats of the thread worker
    worker_thread_stats->static_cou = 0;
    worker_thread_stats->thread_id = id;
    worker_thread_stats->http=0;
    worker_thread_stats->dynamic=0;
    while (1)
    {
        unsigned long* dispatch=(unsigned long*)malloc(sizeof(unsigned long));//data of when the request was added to queue
        unsigned long* arrival=(unsigned long*)malloc(sizeof(unsigned long));//data of how many time it took to take care of the request
        void *pclient;
        pthread_mutex_lock(&mutex); //lock
        if ((pclient = dequeue(dispatch,arrival)) == NULL)
        {
            //relase lock and put thread to sleep until we add a new request to the queue
            pthread_cond_wait(&cond, &mutex);
            pclient = dequeue(dispatch,arrival); //pop a request to handle
        }
        pthread_mutex_unlock(&mutex); //relase lock
        if (pclient != NULL)
        { //can finally take care of the thread
            pthread_mutex_lock(&mutex_workers_num); //lock cur_worker counter
            cur_worker++;
            pthread_mutex_unlock(&mutex_workers_num); //unlock cur_worker counter
            requestHandle((int)pclient,worker_thread_stats,*dispatch,*arrival);
            //add to thread
            pthread_mutex_lock(&mutex_workers_num); //lock cur_worker counter
            cur_worker--;
            pthread_mutex_unlock(&mutex_workers_num); //unlock cur_worker counter
            pthread_cond_signal(&cond_full);
            Close((int)pclient);
        }
        free(dispatch);
        free(arrival);
    }
}
int comp (const void * elem1, const void * elem2) 
{
    int f = *((int*)elem1);
    int s = *((int*)elem2);
    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}
int overload_handle(char* sched_alg,int connfd){
    if(!strcmp(sched_alg,"block")){
        pthread_mutex_lock(&mutex_workers_num);
        pthread_cond_wait(&cond_full, &mutex_workers_num);//wait for queue to stop being full
        pthread_mutex_unlock(&mutex_workers_num);
        return 0;
    }
    else if(!strcmp(sched_alg,"dt")){//deletes the newest request
         Close(connfd);
         return 1;
    }
    else if(!strcmp(sched_alg,"dh")){
        unsigned long* dispatch=(unsigned long*)malloc(sizeof(unsigned long));
        unsigned long* arrival=(unsigned long*)malloc(sizeof(unsigned long));
        pthread_mutex_lock(&mutex);
        dequeue(dispatch,arrival);
        free(dispatch);
        free(arrival);
        pthread_mutex_unlock(&mutex);
        return 0;
    }
    else if(!strcmp(sched_alg,"rd")){
        pthread_mutex_lock(&mutex);
        unsigned long* dispatch=(unsigned long*)malloc(sizeof(unsigned long));
        unsigned long* arrival=(unsigned long*)malloc(sizeof(unsigned long));
        int size = queue_size;
        int ran;
        if(size==0){
            return 0;
        }
        for (int i = 0; i < (size/4); i++)
        {
            ran=rand()%((size)); // number between 0 to size-1
            for (int  j = 0; j < ran; j++)
            {
                void* val=dequeue((dispatch),(arrival));
                if(ran!=j){
                    enqueue(val);
                }
            }
            
        }
        free(dispatch);
        free(arrival);
        pthread_mutex_unlock(&mutex);
        return 0;
    }//else
    printf("ERROR_UNDEFINED_ALGO_EXPERT");
    return 0;
}