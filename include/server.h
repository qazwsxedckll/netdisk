#ifndef __SERVER_H_
#define __SERVER_H_
#include "head.h"
#include "config.h"

typedef struct Node_t
{
    int new_fd;
    struct Node_t* pNext;
}Node_t, *pNode_t;

typedef struct
{
    pNode_t que_head, que_tail;
    int que_capacity;
    int que_size;
    pthread_mutex_t mutex;
}Que_t, *pQue_t;

typedef struct
{
    pthread_t *pth_id;
    int thread_num;
    pthread_cond_t cond;
    Que_t que;
    short start_flag;
}Factory_t, *pFactory_t;

void factory_init(pFactory_t, const Config*, int);

void factory_start(pFactory_t);

void que_init(pQue_t, int);

void que_insert(pQue_t, pNode_t);

int que_get(pQue_t, pNode_t);

int tcp_init(int*, const Config*, int);

int epoll_init(int* epfd, struct epoll_event** evs, int socketFd, const Config* configs, int config_count);

#endif
