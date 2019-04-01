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
    char user_name[USER_LEN + 1];

    //connect to server
    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_addr.s_addr = inet_addr(argv[1]);
    serAddr.sin_port = htons(atoi(argv[2]));

    //user authentication
    ret = 0;
    while (1)
    {
        ret = tran_authen(&socketFd, argv[1], argv[2], user_name, &data, ret);
        if (ret == 0)
        {
            break;
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

