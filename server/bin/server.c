#include "../include/server.h"
#include "../include/config.h"
#include "../include/transmission.h"

extern int exit_flag;

void* transmission(void* pf)
{
    pFactory_t p = (pFactory_t)pf;
    pQue_t pq = &p->que;
    pNode_t pcur;
    int is_get;
    while (1)
    {
        pthread_mutex_lock(&pq->mutex);
        if (pq->que_size == 0)
        {
            if (exit_flag == 1)
            {
                pthread_mutex_unlock(&pq->mutex);
                pthread_exit(NULL);
            }
            pthread_cond_wait(&p->cond, &pq->mutex);
        }
        is_get = que_get(pq, &pcur);
        pthread_mutex_unlock(&pq->mutex);
        if (is_get == 0)
        {
            if (pcur->code == 2)
            {
                if (send_files(pcur->new_fd, pcur->file_name, pcur->file_md5, pcur->file_size))
                {
#ifdef _DEBUG
                    printf("transmission failed\n");
#endif
                }
                else
                {
#ifdef _DEBUG
                    printf("transmission succeed\n");
#endif
                }
            }
            else if (pcur->code == 3)
            {
                if (recv_files(pcur->new_fd, pcur->file_name, pcur->file_size))     //user_name && cur_dir_id
                {
#ifdef _DEBUG
                    printf("transmission failed\n");
#endif
                }
                else
                {
#ifdef _DEBUG
                    printf("transmission succeed\n");
#endif
                }
            }
            free(pcur);
            pcur = NULL;
        }
    }
    return NULL;
}

void factory_init(pFactory_t pf, const Config* configs, int config_count)
{
    char thread_num[5];
    char capacity[5];

    get_conf_value(configs, config_count, "thread_num", thread_num);
    get_conf_value(configs, config_count, "capacity", capacity);
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

int que_get(pQue_t pq, pNode_t* ppNode)
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
        *ppNode = pq->que_head;
        pq->que_head = pq->que_tail = NULL;
        return 0;
    }
    else
    {
        *ppNode = pq->que_head;
        pq->que_head = pq->que_head->pNext;
        return 0;
    }
}

int tcp_init(int* socketFd, const Config* configs, int config_count)
{
    char ip_address[20];
    char port[6];
    char listen_que_length[5];

    get_conf_value(configs, config_count, "ip_address", ip_address);
    get_conf_value(configs, config_count, "port", port);
    get_conf_value(configs, config_count, "listen_que_length", listen_que_length);
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
        return -1;
    }
    listen(*socketFd, atoi(listen_que_length));
#ifdef _DEBUG
    printf("tcp initialized\n");
#endif
    return 0;
}

int epoll_init(int* epfd, struct epoll_event** evs, int socketFd, const Config* configs, int config_count)
{
    char thread_num[5];
    char max_client[5];

    get_conf_value(configs, config_count, "thread_num", thread_num);
    get_conf_value(configs, config_count, "max_client", max_client);

    *epfd = epoll_create(1);
    struct epoll_event event;
    *evs = (struct epoll_event*)calloc(2 + atoi(max_client), sizeof(struct epoll_event));
    event.events = EPOLLIN;
    event.data.fd = socketFd;
    epoll_ctl(*epfd, EPOLL_CTL_ADD, socketFd, &event);
    return 0;
}
