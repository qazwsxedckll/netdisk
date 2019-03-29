#include "../include/server.h"
#include "../include/config.h"
#include "../include/transmission.h"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("wrong arg num\n");
        return -1;
    }

#ifdef _DEBUG
    printf("reading configuration file\n");
#endif
    Config configs[MAX_CONFIG];
    int n = read_conf(argv[1], configs);

    Factory_t f;
    factory_init(&f, configs, n);
    factory_start(&f);

    int socketFd;
    tcp_init(&socketFd, configs, n);

    int new_fd, cmd_len;
    char buf[1000] = {0};
    while (1)
    {
        new_fd = accept(socketFd, NULL, NULL);
        while(1)
        {
            recv_cycle(new_fd, (char*)&cmd_len, sizeof(int));
            recv_cycle(new_fd, buf, cmd_len);
            puts(buf);
        }
    }
}

