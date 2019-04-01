#include "transmission.h"

int send_cycle(int fd, char* data, int send_len)
{
    int total = 0;
    int ret;
    while(total < send_len)
    {
        ret = send(fd, data + total, send_len - total, 0);
        if (ret == -1)
        {
            return -1;
        }
        total = total + ret;
    }
    return 0;
}

int recv_cycle(int fd, char* data, int recv_len)
{
    int total = 0;
    int ret;
    while (total < recv_len)
    {
        ret = recv(fd, data + total, recv_len - total, 0);
        total = total + ret;
    }
    return 0;
}

void print_help()
{
    printf("----------welcome to Evilolipop Netdisk----------\n\n");
    printf("Usage:\n\n");
    printf("list file: ls [file]\n");
    printf("print working directory: pwd\n");
    printf("change directory: cd [path]\n");
    printf("this page: --help\n");
}

int tran_authen(int socketFd, char* user_name, DataPackage* data, int err)
{
    int ret;
    int flag, err_flag = 0;
    do
    {
        flag = 0;
        system("clear");
        if (err == -1)
        {
            printf("wrong username or password\n");
        }
        if (err_flag == 1)
        {
            printf("username too long!\n");
            err_flag = 0;
        }
        printf("Enter username: ");
        fflush(stdout);
        while (1)
        {
            ret = read(STDIN_FILENO, user_name, USER_LEN);
            if (ret >= 20)
            {
                flag = 1;
                err_flag = 1;       //too long err
            }
            else
            {
                break;
            }
        }
    } while (flag == 1);
    user_name[ret - 1] = '\0';

    char* password;
    err_flag = 0;
    while (1)
    {
        if (err_flag == 1)
        {
            system("clear");
            printf("Enter username: %s\n", user_name);
            printf("password too long!\n");
            err_flag = 0;
        }
        password = getpass("Enter password: ");
        if (strlen(password) > 20)
        {
            err_flag = 1;
        }
        else
        {
            break;
        }
    }

    data->data_len = strlen(user_name) + 1;
    strcpy(data->buf, user_name);
    send_cycle(socketFd, (char*)data, data->data_len + sizeof(int));
    data->data_len = strlen(password) + 1;
    strcpy(data->buf, password);
    send_cycle(socketFd, (char*)data, data->data_len + sizeof(int));

    recv_cycle(socketFd, (char*)&data->data_len, sizeof(int));
    if (data->data_len == 0)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int tran_cmd(int socket_fd, DataPackage* data)
{
    data->data_len = read(STDIN_FILENO, data->buf, sizeof(data->buf));
    data->buf[data->data_len - 1] = '\0';

    //verify cmd
    if (strcmp(data->buf, "--help") == 0)
    {
        system("clear");
        print_help();
        return 1;
    }
    else
    {
        int i = 0;
        int space = 0;
        char prefix[10];
        while (1)
        {
            if ((data->buf[i] == ' ' || data->buf[i] == '\0') && space == 0)
            {
                strncpy(prefix, data->buf, i);
                prefix[i] = '\0';
                if (strcmp(prefix, "ls"))
                    if (strcmp(prefix, "cd"))
                        if (strcmp(prefix, "puts"))
                            if (strcmp(prefix, "gets"))
                                if (strcmp(prefix, "remove"))
                                    if (strcmp(prefix, "pwd"))
                                    {
                                        system("clear");
                                        printf("-----$ %s\n", data->buf);
                                        printf("invaild command\n");
                                        return -1;
                                    }
            }
            if (data->buf[i] == ' ')
            {
                space++;
                if (space == 2)
                {
                    system("clear");
                    printf("invaild command\n");
                    printf("-----$ %s\n", data->buf);
                    return -1;
                }
            }
            if (data->buf[i] == '\0')
            {
                break;
            }
            i++;
        }
    }

    send_cycle(socket_fd, (char*)data, data->data_len + 4);
    system("clear");
    printf("-----$ %s\n", data->buf);
    return 0;
}

