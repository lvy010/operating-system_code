#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>

int main()
{
   // close(0);
   // close(2);
   // close(1);
    umask(0);
   // int fd=open("log.txt",O_WRONLY|O_CREAT|O_TRUNC,0666);
   // int fd=open("log.txt",O_WRONLY|O_CREAT|O_APPEND,0666);
    int fd=open("log.txt",O_RDONLY);
    if(fd == -1)
    {
        perror("open");
        exit(1);
    }
    //输入重定向
    dup2(fd,0);
    char outbuffer[64];
    while(1)
    {
        printf("<");
        if(fgets(outbuffer,sizeof(outbuffer),stdin) == NULL) break;
        printf("%s",outbuffer);
    }


    //重定向
 //   dup2(fd,1);
 //  // printf("fd:%d\n",fd);
 //   printf("你好\n");
 //   printf("吃了吗\n");
 //   
 //   //这里必须刷新一下，不然log.txt里面没有内容
 //   fflush(stdout);
 //   close(fd);

    return 0;
}
