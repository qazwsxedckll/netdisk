#include "head.h"
#include "transmission.h"

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf("wrong arg num\n");
        return -1;
    }
    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_addr.s_addr = inet_addr(argv[1]);
    serAddr.sin_port = htons(atoi(argv[2]));
    connect(socketFd, (struct sockaddr*)&serAddr, sizeof(struct sockaddr));
    printf("connected\n");

    while (1)
    {
        tran_cmd(socketFd);
    }
    return 0;
}

