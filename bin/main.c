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
    if (ret)
    {
        return -1;
    }

    //inti tcp
    int socketFd;
    ret = tcp_init(&socketFd, configs, config_count);
    if (ret)
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
    //transmission
    DataPackage data;
    data.data_len = 0;
    char** result;
    //cmd
    char prefix[10] = { 0 };
    char cmd_path[RESULT_LEN] = { 0 };
    //user
    char user_name[USER_NAME_LEN + 1];
    char token[TOKEN_LEN];
    time_t now;
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
#endif                                                                      //get connection code 0 for login, 2 for gets, 3 for puts
                    if (recv_cycle(new_fd, (char*)&data.data_len, sizeof(int))) //4 for invitation code, 5 for regi name, 6 for password
                    {
                        close(new_fd);
                        continue;
                    }
                    if (data.data_len == 0)
                    {
                        //login
                        if (recv_cycle(new_fd, (char*)&data.data_len, sizeof(int))) //get username
                        {
                            close(new_fd);
                            continue;
                        }
                        if (recv_cycle(new_fd, data.buf, data.data_len))
                        {
                            close(new_fd);
                            continue;
                        }
                        strcpy(user_name, data.buf);
                        if (recv_cycle(new_fd, (char*)&data.data_len, sizeof(int))) //get password
                        {
                            close(new_fd);
                            continue;
                        }
                        if (recv_cycle(new_fd, data.buf, data.data_len))
                        {
                            close(new_fd);
                            continue;
                        }
#ifdef _DEBUG
                        printf("username: %s\n", user_name);
                        printf("password: %s\n", data.buf);
#endif
                        ret = user_verify(conn, user_name, data.buf);
                        if (ret)
                        {
                            data.data_len = -1;
                            send_cycle(new_fd, (char*)&data, sizeof(int));
                            close(new_fd);
                            continue;
                        }
                        else
                        {
                            data.data_len = -0;
                            if(send_cycle(new_fd, (char*)&data, sizeof(int)))      //send confirm
                            {
                                close(new_fd);
                                continue;
                            }
                            //send token
                            time(&now);
                            strcpy(token, (char*)&now);
                            strcat(token, user_name);
                            data.data_len = strlen(token) + 1;
                            strcpy(data.buf, token);
                            if (send_cycle(new_fd, (char*)&data, data.data_len + sizeof(int))) //send token
                            {
                                close(new_fd);
                                continue;
                            }
#ifdef _DEBUG
                            printf("token send: %s\n", token);
#endif
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
                        char* root_dir = user_find_root(conn, user_name);
                        strcpy(users[j].root_id, root_dir);
                        strcpy(users[j].cur_dir_id, root_dir);
                        strcpy(users[j].user_name, user_name);
                        strcpy(users[j].token, token);
                        free(root_dir);
                        root_dir = NULL;
                        event.data.fd = users[j].fd;
                        epoll_ctl(epfd, EPOLL_CTL_ADD, users[j].fd, &event);
                        cur_client_num++;
#ifdef _DEBUG
                        printf("cur_client_num: %d\n", cur_client_num);
#endif
                        continue;
                    }

                    if (data.data_len == 2 || data.data_len == 3)
                    {
#ifdef _DEBUG
                        printf("connection for file transmission\n");
#endif

                        if (recv_cycle(new_fd, (char*)&data.data_len, sizeof(int))) //get token
                        {
                            close(new_fd);
                            continue;
                        }
                        if (recv_cycle(new_fd, data.buf, data.data_len))
                        {
                            close(new_fd);
                            continue;
                            }
                        int flag = -1;
#ifdef _DEBUG
                        printf("token recv: %s\n", data.buf);
#endif
                        for (j = 0; j < max_client; j++)
                        {
                            if (strcmp(users[j].token, data.buf) == 0)
                            {
                                flag = 1;                                           //token verified
                                data.data_len = 0;
                                if (send_cycle(new_fd, (char*)&data, sizeof(int)))      //send token verified
                                {
                                    close(new_fd);
                                    break;
                                }
                                if (recv_cycle(new_fd, (char*)&data.data_len, sizeof(int)))     //recv command
                                {
                                    close(new_fd);
                                    break;
                                }
                                if (recv_cycle(new_fd, data.buf, data.data_len))
                                {
                                    close(new_fd);
                                    break;
                                }
                                cmd_interpret(data.buf, prefix, cmd_path);
                                pNode_t pnew = (pNode_t)calloc(1, sizeof(Node_t));
                                if (strcmp(prefix, "gets") == 0)
                                {
                                    flag = 2;
                                    ret = resolve_gets(pnew->file_md5, pnew->file_name, pnew->file_size, cmd_path, conn, users[j].cur_dir_id, users[j].root_id);
                                    if (ret)
                                    {
                                        free(pnew);
                                        pnew = NULL;
                                        flag = -2;
                                        break;
                                    }
                                    pnew->new_fd = new_fd;
                                    pnew->code = 2;
                                    pthread_mutex_lock(&f.que.mutex);
                                    que_insert(&f.que, pnew);
                                    pthread_mutex_unlock(&f.que.mutex);
                                    pthread_cond_signal(&f.cond);
                                    break;
                                }
                                else if (strcmp(prefix, "puts") == 0)
                                {
                                    flag = 3;
                                    ret = resolve_puts(cmd_path, conn, users[j].root_id, users[j].cur_dir_id);
                                    if (ret == -1)
                                    {
                                        free(pnew);
                                        pnew = NULL;
                                        flag = -3;
                                        break;
                                    }
                                    pnew->new_fd = new_fd;
                                    pnew->code = 3;
                                    strcpy(pnew->file_name, users[j].user_name);    //send username via file_name
                                    strcpy(pnew->file_size, users[j].cur_dir_id);                  //send cur_dir_id via file size
                                    pthread_mutex_lock(&f.que.mutex);
                                    que_insert(&f.que, pnew);
                                    pthread_mutex_unlock(&f.que.mutex);
                                    pthread_cond_signal(&f.cond);
                                    break;
                                }
                            }
                        }

                        if (flag == 1)
                        {
                            data.data_len = 1;
                            send_cycle(new_fd, (char*)&data, sizeof(int));  //transmission interrupted;
                            close(new_fd);
                            continue;
                        }
                        if (flag == 2)
                        {
#ifdef _DEBUG
                            printf("start sending\n");
#endif
                            data.data_len = 2;
                            send_cycle(new_fd, (char*)&data, sizeof(int));
                            continue;
                        }
                        if (flag == 3)
                        {
#ifdef _DEBUG
                            printf("start receiving\n");
#endif
                            data.data_len = 3;
                            send_cycle(new_fd, (char*)&data, sizeof(int));
                            continue;
                        }
                        if (flag == -1)
                        {
#ifdef _DEBUG
                            printf("token verification failed\n");
#endif
                            data.data_len = -1;
                            send_cycle(new_fd, (char*)&data, sizeof(int));      //send token verification failed
                            close(new_fd);
                            continue;
                        }
                        if (flag == -2)                                         //gets: cannot get: No such file or directory
                        {
                            data.data_len = -2;
                            send_cycle(new_fd, (char*)&data, sizeof(int));      //send file not exist
                            close(new_fd);
                            continue;

                        }
                        if (flag == -3)                                         //puts: cannot put: File already exist
                        {
                            data.data_len = -3;
                            send_cycle(new_fd, (char*)&data, sizeof(int));      //send file already exsit
                            close(new_fd);
                            continue;

                        }
                    }

                    if (data.data_len == 4)
                    {
#ifdef _DEBUG
                        printf("connection for invitation code\n");
#endif
                        if (recv_cycle(new_fd, (char*)&data.data_len, sizeof(int))) //get code
                        {
                            close(new_fd);
                            continue;
                        }
                        if (recv_cycle(new_fd, data.buf, data.data_len))
                        {
                            close(new_fd);
                            continue;
                        }
                        if (strcmp(data.buf, "cc27") == 0)
                        {
                            data.data_len = 0;
                        }
                        else
                        {
                            data.data_len = -1;
                        }
                        send_cycle(new_fd, (char*)&data, sizeof(int));
                        close(new_fd);
                        continue;
                    }

                    if (data.data_len == 5)
                    {
#ifdef _DEBUG
                        printf("connection for regi name\n");
#endif
                        if (recv_cycle(new_fd, (char*)&data.data_len, sizeof(int)))     //get username
                        {
                            close(new_fd);
                            continue;
                        }
                        if (recv_cycle(new_fd, data.buf, data.data_len))
                        {
                            close(new_fd);
                            continue;
                        }
                        strcpy(user_name, data.buf);
                        MYSQL_RES* res;
                        res = sql_select(conn, "user", "user_name", user_name, 0);
                        if (res == NULL)
                        {
                            mysql_free_result(res);
                            data.data_len = 0;
                            send_cycle(new_fd, (char*)&data, sizeof(int));
                            close(new_fd);
                            continue;
                        }
                        else
                        {
#ifdef _DEBUG
                            printf("username already used\n");
#endif
                            mysql_free_result(res);
                            data.data_len = -1;
                            send_cycle(new_fd, (char*)&data, sizeof(int));
                            close(new_fd);
                            continue;
                        }
                    }

                    if (data.data_len == 6)
                    {
#ifdef _debug
                        printf("connection for regi password\n");
#endif
                        if (recv_cycle(new_fd, (char*)&data.data_len, sizeof(int)))     //get username
                        {
                            close(new_fd);
                            continue;
                        }
                        if (recv_cycle(new_fd, data.buf, data.data_len))
                        {
                            close(new_fd);
                            continue;
                        }
                        strcpy(user_name, data.buf);
                        if (recv_cycle(new_fd, (char*)&data.data_len, sizeof(int)))     //get password
                        {
                            close(new_fd);
                            continue;
                        }
                        if (recv_cycle(new_fd, data.buf, data.data_len))
                        {
                            close(new_fd);
                            continue;
                        }

                        ret = sql_insert_user_trans(conn, user_name, data.buf, "1", 0, user_name, 0, "");
                        if (ret == -1)
                        {
                            data.data_len = -1;
                        }
                        else if (ret == 0)
                        {
#ifdef _debug
                            printf("user created\n");
#endif
                            data.data_len = 0;
                        }
                        send_cycle(new_fd, (char*)&data, sizeof(int));
                        close(new_fd);
                        continue;
                    }

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
                    ret = recv_cycle(users[j].fd, (char*)&data.data_len, sizeof(int));
                    if (ret)
                    {
#ifdef _DEBUG
                        printf("client disconnected\n");
#endif
                        close(users[j].fd);
                        users[j].fd = 0;
                        epoll_ctl(epfd, EPOLL_CTL_DEL, users[j].fd, &event);
                        cur_client_num--;
                        break;
                    }
                    if (recv_cycle(users[j].fd, data.buf, data.data_len))
                    {
                        users[j].fd = 0;
                        close(users[j].fd);
                        continue;
                    }
#ifdef _DEBUG
                    printf("received form client: %s\n", data.buf);
#endif
                    //resolve command
                    cmd_interpret(data.buf, prefix, cmd_path);
                    /*return:
                     *  1 for normal cmd
                     *  3 for rm success
                     *  -1 for ls error
                     *  -2 for cd error
                     *  -3 for rm error
                     *  -4 for mkdir error
                     *  -10 for account cancellation*/
                    if (strcmp(prefix, "ls") == 0)
                    {
                        ret = resolve_ls(&result, &res_lines, cmd_path, conn, users[j].cur_dir_id, users[j].root_id);
                    }
                    else if (strcmp(prefix, "pwd") == 0)
                    {
                        ret = resolve_pwd(&result, &res_lines, conn, users[j].cur_dir_id, strlen(users[j].user_name));
                    }
                    else if (strcmp(prefix, "cd") == 0)
                    {
                        if (strlen(cmd_path) == 0)
                        {
                            ret = -2;
                        }
                        else
                        {
                            ret = resolve_cd(&result, &res_lines, cmd_path, conn, users[j].cur_dir_id, users[j].root_id);
                        }
                    }
                    else if (strcmp(prefix, "rm") == 0)
                    {
                        ret = resolve_rm(cmd_path, 0, conn, users[j].user_name, users[j].root_id, users[j].cur_dir_id);
                    }
                    else if (strcmp(prefix, "mkdir") == 0)
                    {
                        ret = resolve_mkdir(&result, &res_lines, users[j].user_name, cmd_path, conn, users[j].cur_dir_id, users[j].root_id);
                    }

                    //send result to client
                    if (ret < 0)        //errors
                    {
                        if (ret == -1)      //ls error
                        {
                            strcpy(data.buf, "ls: cannot access: No such file or directory");
                        }
                        if (ret == -2)     //cd error
                        {
                            strcpy(data.buf, "cd: cannot access: No such directory");
                        }
                        if (ret == -3)      //rm error
                        {
                            strcpy(data.buf, "rm: cannot remove: No such file");
                        }
                        if (ret == -4)      //mkdir error
                        {
                            strcpy(data.buf, "mkdir: cannot make directory");
                        }
                        if (ret == -10)      //mkdir error
                        {
                            strcpy(data.buf, "你号没了，重新注册吧");
                            data.data_len = strlen(data.buf) + 1;
                            send_cycle(users[j].fd, (char*)&data, data.data_len + sizeof(int));
                            close(users[j].fd);
                            users[j].fd = 0;
                            epoll_ctl(epfd, EPOLL_CTL_DEL, users[j].fd, &event);
                            cur_client_num--;
                            break;
                        }
                        data.data_len = strlen(data.buf) + 1;
                        if (send_cycle(users[j].fd, (char*)&data, data.data_len + sizeof(int)))
                        {
                            users[j].fd = 0;
                            close(users[j].fd);
                            break;
                        }
                    }
                    else
                    {
                        if (ret == 1)      //normal send
                        {
                            for (i = 0; i < res_lines; i++)
                            {
                                strcpy(data.buf, result[i]);
                                data.data_len = strlen(data.buf) + 1;
                                if (send_cycle(users[j].fd, (char*)&data, data.data_len + sizeof(int)))
                                {
                                    users[j].fd = 0;
                                    close(users[j].fd);
                                    break;
                                }
                            }
                        }
                        if (ret == 3)       //remove success
                        {
                            strcpy(data.buf, cmd_path);
                            strcat(data.buf, "\tis removed");
                            data.data_len = strlen(data.buf) + 1;
                            if (send_cycle(users[j].fd, (char*)&data, data.data_len + sizeof(int)))
                            {
                                users[j].fd = 0;
                                close(users[j].fd);
                                break;
                            }
                        }
                    }
                    //send end of transmission
                    data.data_len = 0;
                    if (send_cycle(users[j].fd, (char*)&data, sizeof(int)))
                    {
                        users[j].fd = 0;
                        close(users[j].fd);
                    }
                    break;
                }
            }
        }
    }
    mysql_close(conn);
    return 0;
}

