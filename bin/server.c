#include "../include/server.h"
#include "../include/config.h"

void* transmission(void* pf)
{
    pFactory_t p = (pFactory_t)pf;
    printf("start\n");
    pthread_cond_wait(&p->cond, &p->que.mutex);
    return NULL;
}

void factory_init(pFactory_t pf, const Config* configs, int n)
{
    char thread_num[5];
    char capacity[5];

    get_conf_value(configs, n, "thread_num", thread_num);
    get_conf_value(configs, n, "capacity", capacity);
    memset(pf, 0, sizeof(Factory_t));
    pf->pth_id = (pthread_t*)calloc(atoi(thread_num), sizeof(pthread_t));
    pf->thread_num = atoi(thread_num);
    pthread_cond_init(&pf->cond, NULL);
    que_init(&pf->que, atoi(capacity));
}

void factory_start(pFactory_t pf)
{
    int i;
    if (pf->start_flag == 0)
    {
        for (i = 0; i < pf->thread_num; i++)
        {
#ifdef _DEBUG
            printf("pthread_create %d\n", i);
#endif
            pthread_create(pf->pth_id + i, NULL, transmission, pf);
        }
        pf->start_flag = 1;
    }
}

void que_init(pQue_t pq, int capacity)
{
    memset(pq, 0, sizeof(Que_t));
    pq->que_capacity = capacity;
    pthread_mutex_init(&pq->mutex, NULL);
}

void que_insert(pQue_t pq, pNode_t new_node)
{
    if (pq->que_head == NULL)
    {
        pq->que_head = pq->que_tail = new_node;
    }
    else
    {
        pq->que_tail->pNext = new_node;
        pq->que_tail = new_node;
    }
}

int que_get(pQue_t pq, pNode_t pNode)
{
    if (pq->que_head == NULL)
    {
#ifdef _DEBUG
        printf("que is emtpy\n");
#endif
        return -1;
    }
    else if (pq->que_head == pq->que_tail)
    {
        pNode = pq->que_head;
        pq->que_head = pq->que_tail = NULL;
        return 0;
    }
    else
    {
        pNode = pq->que_head;
        pq->que_head = pq->que_head->pNext;
        return 0;
    }
}

int* tcp_init(int* socketFd, const Config* configs, int n)
{
    char ip_address[20];
    char port[6];
    char max_tcp_connection[5];

    get_conf_value(configs, n, "ip_address", ip_address);
    get_conf_value(configs, n, "port", port);
    get_conf_value(configs, n, "max_tcp_connection", max_tcp_connection);
#ifdef _DEBUG
    printf("ip_adress: %s, port: %s\n", ip_address, port);
#endif

    int ret;
    *socketFd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serAddr;
    memset(&serAddr, 0, sizeof(serAddr));
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(atoi(port));
    serAddr.sin_addr.s_addr = inet_addr(ip_address);
    ret = bind(*socketFd, (struct sockaddr*)&serAddr, sizeof(struct sockaddr));
    if (ret == -1)
    {
#ifdef _DEBUG
        printf("bind failed\n");
#endif
        return NULL;
    }
    listen(*socketFd, atoi(max_tcp_connection));
    return socketFd;
}
