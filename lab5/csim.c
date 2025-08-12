#include "cachelab.h"
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
typedef struct
{
    int valid;
    int tag;
    char *data;

    int time; // Last Recently Used counter
} cache_line_t;

typedef struct
{
    int s;
    int E;
    int b;
    cache_line_t ***cache_lines;
}cache_t;

cache_t Cache;
int Hits = 0, Misses = 0, Evictions = 0;
static const int Max_time = 1 << 31;


char filename[100];
int s, E, b;
int verbose = 0;

void initLines()
{
    Cache.cache_lines = (cache_line_t ***)malloc(sizeof(cache_line_t **) * (1 << Cache.s));
    for (int i = 0; i < (1 << Cache.s); i++)
    {
        Cache.cache_lines[i] = (cache_line_t **)malloc(sizeof(cache_line_t *) * Cache.E);
    }
    for (int i = 0; i < (1 << Cache.s); i++)
    {
        for (int j = 0 ; j < Cache.E; j++)
            Cache.cache_lines[i][j] = (cache_line_t *)malloc(sizeof(cache_line_t));
    }

    for (int i = 0; i < (1 << Cache.s); i++){
        for (int j = 0; j < Cache.E; j++){
            Cache.cache_lines[i][j]->valid = 0;
            Cache.cache_lines[i][j]->tag = -1;
            Cache.cache_lines[i][j]->time = Max_time;
            Cache.cache_lines[i][j]->data = (char*)malloc(sizeof(char) * (1 << Cache.b));
        }
    }
}

void initCache(int s, int E, int b)
{
    Cache.s = s;
    Cache.E = E;
    Cache.b = b;
    initLines();
}

void freeCache()
{
    for (int i = 0 ; i < (1 << Cache.s); i++)
    {
        for (int j = 0 ; j < Cache.E; j++)
            free(Cache.cache_lines[i][j]->data);
    }
    for (int i = 0 ; i < (1 << Cache.s); i++)
        for (int j = 0; j < Cache.E; j++)
            free(Cache.cache_lines[i][j]);

    for (int i = 0 ; i < (1 << Cache.s); i++)
        free(Cache.cache_lines[i]);
    
    free(Cache.cache_lines);
}

void getoptions(int argc, char* argv[])
{
    char opt;

    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1 )
    {
        switch(opt)
        {
            case 'h':
                printf("sorry, no help message\n");
                exit(0);
            case 'v':
                verbose = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                strcpy(filename, optarg);
                break;
            default:
                printf("wrong argument\n");
                exit(1);
        }
    }
}

void simTrace(int s, int E, int b, char *filename)
{
    FILE *file_ptr = fopen(filename, "r");
    if (!file_ptr) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }
    char operation;
    unsigned adress;
    int size;

    while (fscanf(file_ptr, " %c %x,%d", &operation, &adress, &size) != EOF)
    {
        if (operation == 'I')
            continue;
        int set_index = (adress >> b) & ((1 << s) - 1);    //取出地址的set index部分
        int tag = adress >> (b + s);                        //取出地址的tag部分

        int hit_flag = 0;
        int empty_line_index = -1;
        int lru_index = 0;
        int max_time = -1;


        for (int i = 0; i < E; i++) {
            if (Cache.cache_lines[set_index][i]->valid)
                Cache.cache_lines[set_index][i]->time++;
        }
        
        for (int i = 0; i < E; i++)
        {
            if (Cache.cache_lines[set_index][i]->valid)                     //遍历该set中的每个有效的cache line
            {
                if (Cache.cache_lines[set_index][i]->tag == tag)            //如果该行有效且tag匹配
                {
                    hit_flag = 1;                                          //命中
                    Cache.cache_lines[set_index][i]->time = 0;             //命中后将该行的time设为0，表示最近使用过
                    Hits++;
                    if (verbose)                                           //如果指令要求打印详细信息，则打印
                        printf("%c %x, %d hit\n", operation, adress, size);
                    break;
                }
                else                                                         //如果该行有效但tag不匹配       
                {  
                    if (Cache.cache_lines[set_index][i]->time > max_time)   //找出time最大的行，即最久未使用的行
                    {
                        max_time = Cache.cache_lines[set_index][i]->time;
                        lru_index = i;                                      //记录最久未使用的行的索引
                    }
                    
                }
            }
            else if (empty_line_index == -1)                                //如果该行无效且还没有找到空行,记录该行的索引
            {
                empty_line_index = i;
            }
        }

        if (!hit_flag)                          //如果把这个set遍历完了还是未命中，则需要替换
        {
            Misses++;                           
            if (empty_line_index != -1)         //如果有空行
            {
                Cache.cache_lines[set_index][empty_line_index]->valid = 1;
                Cache.cache_lines[set_index][empty_line_index]->tag = tag;
                Cache.cache_lines[set_index][empty_line_index]->time = 0;
                if (verbose)
                    printf("%c %x, %d miss\n", operation, adress, size);
            }
            else                                            //如果没有空行，lru替换
            {
                Evictions++;
                Cache.cache_lines[set_index][lru_index]->tag = tag;
                Cache.cache_lines[set_index][lru_index]->time = 0;
                if (verbose)
                    printf("%c %x, %d miss eviction\n", operation, adress, size);
            }
        }

        if (operation == 'M')              //如果是修改操作，则需要再命中一次
        {
            Hits++;
            if (verbose)
                printf("%c %x, %d hit\n", operation, adress, size);
        }
    }
}



int main(int argc, char **argv)
{
    getoptions(argc, argv);
    initCache(s, E, b);
    simTrace(s, E, b, filename);
    printSummary(Hits, Misses, Evictions);
    freeCache();
    return 0;
}
