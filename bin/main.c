#include "../include/server.h"
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
    int n = read_conf(argv[1], configs);

    //create threads
    Factory_t f;
    factory_init(&f, configs, n);
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
    ret = tcp_init(&socketFd, configs, n);
    if (ret == -1)
    {
        return -1;
    }

    int new_fd, res_lines, i;
    DataPackage data;
    char** result;
    while (1)
    {
        new_fd = accept(socketFd, NULL, NULL);
        int dir_id = 5; //put client in root dir 
        while(1)
        {
            recv_cycle(new_fd, (char*)&data.data_len, sizeof(int));
            recv_cycle(new_fd, data.buf, data.data_len);
#ifdef _DEBUG
            printf("received form client: %s\n", data.buf);
#endif

            ret = cmd_interpret(&result, &res_lines, data.buf, conn, dir_id);
            if (ret == -1)
            {
                strcpy(data.buf, "ls: cannot access: No such file or directory");
                data.data_len = strlen(data.buf) + 1;
                send_cycle(new_fd, (char*)&data, data.data_len + sizeof(int));
            }
            else
            {
                for (i = 0; i < res_lines; i++)
                {
                    strcpy(data.buf, result[i]);
                    data.data_len = strlen(data.buf) + 1;
                    send_cycle(new_fd, (char*)&data, data.data_len + sizeof(int));
                }
            }
            //send end of transmission
            data.data_len = 0;
            send_cycle(new_fd, (char*)&data, sizeof(int));
        }
    }
    mysql_close(conn);
    return 0;
}

