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

    GetsInfo getsinfo;
    strcpy(getsinfo.ip_address, argv[1]);
    strcpy(getsinfo.port, argv[2]);
    pthread_t tran_thread;

    int socketFd;

    //user authentication
    ret = 0;
    while (1)
    {
        ret = tran_authen(&socketFd, argv[1], argv[2], user_name, &data, ret);
        if (ret == 0)
        {
            strcpy(getsinfo.token, data.buf);
            break;
        }
    }

    system("clear");
    print_help();
    while (1)
    {
        data.data_len = read(STDIN_FILENO, data.buf, sizeof(data.buf));
        data.buf[data.data_len - 1] = '\0';
        
        ret = cmd_interpret(&data);
        if (ret == 1 || ret == -1)      //0 for normal, 1 for help page, -1 for error
        {
            continue;
        }
        else if (ret == 2)          // 2 for gets
        {
            strcpy(getsinfo.cmd, data.buf);
            pthread_create(&tran_thread, NULL, get_files, &getsinfo);
        }
        else
        {
            tran_cmd(socketFd, &data);
        }
    }
    return 0;
}

