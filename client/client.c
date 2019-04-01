#include "head.h"
#include "transmission.h"

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf("wrong arg num\n");
        return -1;
    }

    int ret;

    DataPackage data;
    char user_name[USER_LEN]; 
    char *password;

    //connect to server
    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_addr.s_addr = inet_addr(argv[1]);
    serAddr.sin_port = htons(atoi(argv[2]));
    ret = connect(socketFd, (struct sockaddr*)&serAddr, sizeof(struct sockaddr));
    if (ret == -1)
    {
        printf("connect failed\n");
        return -1;
    }

    //user authentication
    int flag;
    int err_flag = 0;
    while (1)
    {
        do
        {
            flag = 0;
            system("clear");
            if (err_flag == 1)
            {
                PRINT_FONT_RED;
                printf("user name too long!\n");
                PRINT_FONT_WHI;
                err_flag = 0;
            }
            if (err_flag == 2)
            {
                PRINT_FONT_RED;
                printf("username do not exist\n");
                PRINT_FONT_WHI;
                err_flag =0;
            }
            printf("Enter username: ");
            fflush(stdout);
            while (1)
            {
                ret = read(STDIN_FILENO, user_name, sizeof(user_name));
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
        data.data_len = strlen(user_name) + 1;
        strcpy(data.buf, user_name);
        send_cycle(socketFd, (char*)&data, data.data_len + 4);
        recv_cycle(socketFd, (char*)&data.data_len, sizeof(int));
        if (data.data_len == 0)
        {
            break;
        }
        else
        {
            err_flag = 2;       //wrong username err
        }
    }

    flag = 0;
    while (1)
    {
        if (flag == 1)
        {
            system("clear");
            printf("Enter username: %s\n", user_name);
            if (err_flag == 2)
            {
                PRINT_FONT_RED;
                printf("wrong password\n");
                PRINT_FONT_WHI;
                err_flag =0;
            }
        }
        password = getpass("Enter password: ");
        data.data_len = strlen(password) + 1;
        strcpy(data.buf, password);
        send_cycle(socketFd, (char*)&data, data.data_len + 4);
        recv_cycle(socketFd, (char*)&data.data_len, sizeof(int));
        if (data.data_len == 0)
        {
            break;
        }
        else
        {
            flag = 1;
            err_flag = 2;
        }
    }

    system("clear");
    print_help();

    while (1)
    {
        ret = tran_cmd(socketFd, &data);
        if (ret == 1 || ret == -1)
        {
            continue;
        }
        while (1)
        {
            recv_cycle(socketFd, (char*)&data.data_len, sizeof(int));
            if (data.data_len == 0)
            {
                break;
            }
            recv_cycle(socketFd, data.buf, data.data_len);
            printf("%s\n", data.buf);
        }
    }
    return 0;
}

