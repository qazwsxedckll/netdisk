#include "../include/client.h"

int connect_server(int* socketFd, const char* ip, const char* port)
{
    int ret;
    *socketFd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_addr.s_addr = inet_addr(ip);
    serAddr.sin_port = htons(atoi(port));
    ret = connect(*socketFd, (struct sockaddr*)&serAddr, sizeof(struct sockaddr));
    if (ret == -1)
    {
        printf("connection failed\n");
        return -1;
    }
    return 0;
}

int login_page(int flag)
{
    system("clear");
    if (flag == 1)
    {
        printf("register success\n");
    }
    else if (flag == -1)
    {
        printf("register failed\n");
    }
    printf("Please enter a num to continue... \n");
    printf("1.\tLogin\n");
    printf("2.\tRegister\n");
    printf("\n");
    printf("0.\tExit\n");
    int i, c;
    i = getchar();
    while ((c = getchar()) != '\n');
    return i;
}

void print_help()
{
    printf("----------welcome to Evilolipop Netdisk----------\n\n");
    printf("Usage:\n\n");
    printf("list file:                 ls [<file>]\n");
    printf("print working directory:   pwd\n");
    printf("change directory:          cd <path>\n");
    printf("download file:             gets <file>, directory not supported\n");
    printf("upload file:               puts <file>, directory not supported\n");
    printf("remove file:               rm <file>, directory supported~~\n");
    printf("make directory:            mkdir <file>, can only create in current directory\n");
    printf("this page:                 --help\n");
}

int cmd_interpret(const DataPackage* data)
{
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
                if (strcmp(prefix, "ls") && strcmp(prefix, "cd") && strcmp(prefix, "pwd")
                    && strcmp(prefix, "puts") && strcmp(prefix, "gets") && strcmp(prefix, "rm")
                    && strcmp(prefix, "mkdir"))
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
                if (space == 0)
                {
                    if (!strcmp(prefix, "cd") || !strcmp(prefix, "puts")
                        || !strcmp(prefix, "gets") || !strcmp(prefix, "rm")
                        || !strcmp(prefix, "mkdir"))
                    {
                        system("clear");
                        printf("please enter file path\n");
                        printf("-----$ %s\n", data->buf);
                        return -1;
                    }
                }
                break;
            }
            i++;
        }

        if (strcmp(prefix, "gets") == 0)
        {
            system("clear");
            printf("-----$ %s\n", data->buf);
            return 2;
        }
        if (strcmp(prefix, "puts") == 0)
        {
            system("clear");
            printf("-----$ %s\n", data->buf);
            return 3;
        }
        return 0;
    }
}
