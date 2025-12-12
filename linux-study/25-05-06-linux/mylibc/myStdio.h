#pragma once

#include<stdio.h>    
#include<string.h>    
#include<sys/types.h>    
#include<sys/stat.h>    
#include<fcntl.h>    
#include<unistd.h>    
#include<stdlib.h>    
#include<assert.h>
#include<errno.h>

#define SIZE 1024
#define SYNC_NOW 1
#define SYNC_LINE 2
#define SYNC_FULL 3

typedef struct _FILE{
    int flags;//刷新方式
    int fileno;
    int capacity;//buffer容量
    int size;//buffer当前使用量
    char buffer[SIZE];
}_FILE;


_FILE* _fopen(const char* path_name,const char* mode);
void _fwrite(const void* ptr,int num,_FILE* fp);
void _fclose(_FILE* fp);
void _fflush(_FILE* fp);



