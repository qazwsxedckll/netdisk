#include "transmission.h"

int send_cycle(int fd, char *p, int send_len)
{
    int total = 0;
    int ret;
    while(total < send_len)
    {
        ret = send(fd, p + total, send_len - total, 0);
        if (ret == -1)
        {
            return -1;
        }
        total = total + ret;
    }
    return 0;
}

int recv_cycle(int fd, char* p, int recv_len)
{
    int total = 0;
    int ret;
    while (total < recv_len)
    {
        ret = recv(fd, p + total, recv_len - total, 0);
        total = total + ret;
    }
    return 0;
}

void print_help()
{
    printf("----------welcome to Evilolipop Netdisk----------\n\n");
    printf("Usage:\n\n");
    printf("list file: ls [absolute path]\n");
    printf("print working directory: pwd\n");
    printf("this page: --help\n");
}

int tran_cmd(int socket_fd, DataPackage* data)
{
    data->data_len = read(STDIN_FILENO, data->buf, sizeof(data->buf));
    data->buf[data->data_len - 1] = '\0';
    if (strcmp(data->buf, "--help") == 0)
    {
        system("clear");
        print_help();
        return 1;
    }
    send_cycle(socket_fd, (char*)data, data->data_len + 4);
    system("clear");
    printf("-----$ %s\n", data->buf);
    return 0;
}

