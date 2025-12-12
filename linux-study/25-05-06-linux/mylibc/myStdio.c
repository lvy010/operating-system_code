#include"myStdio.h"



_FILE* _fopen(const char* path_name,const char* mode)
{
    assert(path_name);
    int flags=0;
    int defaultmode=0666;

    if(strcmp(mode,"r") == 0)
    {
        flags|=O_RDONLY;
    }
    else if(strcmp(mode,"w") == 0)
    {
        flags|=(O_WRONLY|O_CREAT|O_TRUNC);
    }
    else if(strcmp(mode,"a") == 0)
    {
        flags|=(O_WRONLY|O_CREAT|O_APPEND);
    }

    
    int fd=0;

    if(flags & O_RDONLY)
    {
        fd=open(path_name,flags);
    }
    else
    {
        fd=open(path_name,flags,defaultmode);
    }
    if(fd < 0)
    {
        const char*err=strerror(errno);
        write(2,err,strlen(err));
        return NULL;//这就是为什么创建文件失败，返回NULL
    }

    _FILE* fp=(_FILE*)malloc(sizeof(_FILE));
    assert(fp);

    fp->flags=SYNC_LINE;
    fp->fileno=fd;
    fp->capacity=SIZE;
    fp->size=0;
    memset(fp->buffer,0,SIZE);

    return fp;

}



void _fwrite(const void* ptr,int num,_FILE* fp)
{
  
    //写到缓冲区里
    memcpy(fp->buffer+fp->size,ptr,num);
    fp->size+=num;

    //判断是否要刷新
    if(fp->flags & SYNC_NOW)
    {
        write(fp->fileno,fp->buffer,fp->size);
        fp->size=0;//清空缓冲区
    }
    else if(fp->flags & SYNC_LINE)
    {
        if(fp->buffer[fp->size-1] == '\n')
        {
            write(fp->fileno,fp->buffer,fp->size);
            fp->size=0;
        }
    }
    else if(fp->flags & SYNC_FULL)
    {
        if(fp->size == fp->capacity)
        {
            write(fp->fileno,fp->buffer,fp->size);
            fp->size=0;
        }
    }
}


void _fflush(_FILE* fp)
{
    if(fp->size > 0) write(fp->fileno,fp->buffer,fp->size);
    fsync(fp->fileno);
    fp->size=0;
}


void _fclose(_FILE* fp)
{
    _fflush(fp);
    close(fp->fileno);
}
