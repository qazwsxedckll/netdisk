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
    int flag2 = 0;
    do
    {
        flag = 0;
        system("clear");
        if (flag2 == 1)
        {
            PRINT_FONT_RED
            printf("user name too long!\n");
            PRINT_FONT_WHI;
        }
        printf("Enter user_name: ");
        fflush(stdout);
        while (1)
        {
            ret = read(STDIN_FILENO, user_name, sizeof(user_name));
            if (ret >= 20)
            {
                flag = 1;
                flag2 = 1;
            }
            else
            {
                break;
            }
        }
    } while (flag == 1);
    user_name[ret - 1] = '\0';
    password = getpass("Enter password: ");
    puts(user_name);

    print_help();

    return 0;
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

