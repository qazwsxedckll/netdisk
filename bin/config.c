#include "../include/config.h"

int read_conf(const char* path, Config* configs)
{
    FILE* fp = fopen(path, "r");
    char buf[100];

    int i, k, j = 0;
    int keyLen;
    while (fgets(buf, 100, fp) != NULL)
    {
        //remove comments and blank lines
        if (buf[0] == '#' || buf[0] == '\n')
        {
            continue;
        }
        else
        {
            i = 0;
            while(buf[i] != '\0')
            {
                //store key
                if (buf[i] == ' ')
                {
                    configs[j].key = (char*)malloc(i + 1);
                    strncpy(configs[j].key, buf, i);
                    configs[j].key[i] = '\0';
                }
                //store value
                else if (buf[i] == '\n')
                {
                    keyLen = strlen(configs[j].key);
                    configs[j].value = (char*)malloc(i - keyLen);
                    for (k = 0; k < i - keyLen; k++)
                    {
                        configs[j].value[k] = buf[k + keyLen + 1];
                    }
                    configs[j].value[k - 1] = '\0';
                }
                i++;
            }
        }
        j++;
    }
    return j;
}

int get_conf_value(const Config* configs,int n, const char* key, char* value)
{
    for (int i = 0; i < n; i++)
    {
        if (!strcmp(configs[i].key, key))
        {
            strcpy(value, configs[i].value);
            return 0;
        }
    }
#ifdef _DEBUG
    printf("config %s not found\n", key);
#endif
    return -1;
}

