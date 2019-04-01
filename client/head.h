#ifndef __FUN_H__
#define __FUN_H__

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define _DEBUG
#define CMD_LEN 1000
#define USER_LEN 20

/*设置输出前景色*/
#define PRINT_FONT_RED  printf("\033[31m"); //红色
#define PRINT_FONT_CYA  printf("\033[36m"); //青色
#define PRINT_FONT_WHI  printf("\033[37m"); //白色

#endif

