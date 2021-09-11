#ifndef __REQUEST_H__
struct stats{
    int thread_id; 
    int http;
    int static_cou;
    int dynamic;
};
typedef struct stats stats_t;


void requestHandle(int fd,stats_t* thread_stat, unsigned long dispatch,unsigned long arival);

#endif
