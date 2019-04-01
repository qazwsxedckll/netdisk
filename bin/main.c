#include "../include/server.h"
#include "../include/user.h"
#include "../include/config.h"
#include "../include/transmission.h"
#include "../include/sql.h"
#include "../include/cmd.h"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("wrong arg num\n");
        return -1;
    }

    int ret;

    //read configuration
#ifdef _DEBUG
    printf("reading configuration file\n");
#endif
    Config configs[MAX_CONFIG];
    int config_count = read_conf(argv[1], configs);
    char s_thread_num[5];
    char s_max_client[5];
    get_conf_value(configs, config_count, "thread_num", s_thread_num);
    get_conf_value(configs, config_count, "max_client", s_max_client);
    const int thread_num = atoi(s_thread_num);
    const int max_client = atoi(s_max_client);
#ifdef _DEBUG
    printf("thread_num: %d\n", thread_num);
    printf("max_client: %d\n", max_client);
#endif

    //create threads
    Factory_t f;
    factory_init(&f, configs, config_count);
    factory_start(&f);

    //connect database
    MYSQL* conn;
    ret = sql_connect(&conn);
    if (ret == -1)
    {
        return -1;
    }

    //inti tcp
    int socketFd;
    ret = tcp_init(&socketFd, configs, config_count);
    if (ret == -1)
    {
        return -1;
    }

    //epoll set up
    int epfd, new_fd;
    Users users = (Users)malloc(max_client * sizeof(User));
    memset(users, 0, sizeof(User) * max_client);
    struct epoll_event event, *evs;
    event.events = EPOLLIN;
    epoll_init(&epfd, &evs, socketFd, configs, config_count);

    int res_lines, i, j, ready_fd_num, cur_client_num = 0;
    DataPackage data;
    data.data_len = 0;
    char** result;
    char user_name[USER_NAME_LEN + 1];
    while (1)
    {
        ready_fd_num = epoll_wait(epfd, evs, 1 + max_client, -1);
        for (i = 0; i < ready_fd_num; i++)
        {
            if (evs[i].data.fd == socketFd)         //new client coming in
            {
                if (cur_client_num <= max_client)
                {
                    new_fd = accept(socketFd, NULL, NULL);
#ifdef _DEBUG
                    printf("incoming connection\n");
#endif
                    recv_cycle(new_fd, (char*)&data.data_len, sizeof(int));
                    recv_cycle(new_fd, data.buf, data.data_len);
                    strcpy(user_name, data.buf);
                    recv_cycle(new_fd, (char*)&data.data_len, sizeof(int));
                    recv_cycle(new_fd, data.buf, data.data_len);
#ifdef _DEBUG
                    printf("username: %s\n", user_name);
                    printf("password: %s\n", data.buf);
#endif
                    /* ret = user_verify(); */
                    ret = 0;
                    if (ret == -1)
                    {
#ifdef _DEBUG
                        printf("verifivation failed\n");
#endif
                        data.data_len = -1;
                        send_cycle(new_fd, (char*)&data, sizeof(int));
                        continue;
                    }
                    else
                    {
#ifdef _DEBUG
                        printf("verifivation success\n");
#endif
                        data.data_len = 0;
                        send_cycle(new_fd, (char*)&data, sizeof(int));
                    }

                    for (j = 0; j < max_client; j++)
                    {
                        if (users[j].fd == 0)
                        {
                            users[j].fd = new_fd;
                            break;
                        }
                    }
                    //sql root dir by user_name
                    strcpy(users[j].root_id, "9");
                    strcpy(users[j].cur_dir_id, "9");
                    strcpy(users[j].user_name, user_name);
                    event.data.fd = users[j].fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, users[j].fd, &event);
                    cur_client_num++;
                    continue;
                }
                else
                {
#ifdef _DEBUG
                    printf("connection rejected: reach max client\n");
#endif
                    continue;
                }
            }

            for (j = 0; j < max_client; j++)        //command coming in
            {
                if (evs[i].data.fd == users[j].fd)
                {
                    recv_cycle(users[j].fd, (char*)&data.data_len, sizeof(int));
                    recv_cycle(users[j].fd, data.buf, data.data_len);
#ifdef _DEBUG
                    printf("received form client: %s\n", data.buf);
#endif
                    ret = cmd_interpret(&result, &res_lines, conn, data.buf, users[j].cur_dir_id, users[j].root_id);
                    if (ret == -1)      //ls error
                    {
                        strcpy(data.buf, "ls: cannot access: No such file or directory");
                        data.data_len = strlen(data.buf) + 1;
                        send_cycle(users[j].fd, (char*)&data, data.data_len + sizeof(int));
                    }
                    else if (ret == -2)     //cd error
                    {
                        strcpy(data.buf, "cd: cannot access: No such directory");
                        data.data_len = strlen(data.buf) + 1;
                        send_cycle(users[j].fd, (char*)&data, data.data_len + sizeof(int));
                    }
                    else if (ret == 2)      //cd success
                    {
                        strcpy(data.buf, result[0]);
                        data.data_len = strlen(data.buf) + 1;
                        send_cycle(users[j].fd, (char*)&data, data.data_len + sizeof(int));
                    }
                    else
                    {
                        for (i = 0; i < res_lines; i++)
                        {
                            strcpy(data.buf, result[i]);
                            data.data_len = strlen(data.buf) + 1;
                            send_cycle(users[j].fd, (char*)&data, data.data_len + sizeof(int));
                        }
                    }
                    //send end of transmission
                    data.data_len = 0;
                    send_cycle(users[j].fd, (char*)&data, sizeof(int));
                    break;
                }
            }
        }
    }
    mysql_close(conn);
    return 0;
}

