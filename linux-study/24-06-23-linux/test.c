#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<assert.h>
#include<unistd.h>
#include<string.h>

#define FILE_NAME "bite.txt"
int main()
{
   // int fd=open(FILE_NAME,O_WRONLY|O_CREAT|O_TRUNC,0666);
    int fd=open(FILE_NAME,O_RDONLY);
    assert(fd != -1);

 //   //写
 //   char buffer[64];
 //   int cnt=5;
 //   while(cnt)
 //   {
 //      sprintf(buffer,"%s:%d\n","I like linux!",cnt--);
 //      write(fd,buffer, strlen(buffer));
 //   }
   
    //读
    char outbuff[1024];
    ssize_t num=read(fd,outbuff,sizeof(outbuff)-1);
    if(num > 0) outbuff[num]=0;
    printf("%s",outbuff);
    
    
    close(fd);


    return 0;
}
