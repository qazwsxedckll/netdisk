#ifndef __CONFIG_H__
#define __CONFIG_H__
#include "head.h"

#define CONFIG_LEN 50
#define MAX_CONFIG 50

typedef struct
{
    char* key;
    char* value;
}Config;

int read_conf(const char*, Config*);

int get_conf_value(const Config*, int, const char*, char*);
#endif
