#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>


#define FILE_NAME(number) "log.txt"#number

//每一个宏，对应的数值，只有一个比特位是1，彼此位置不能重叠
#define ONE (1<<0)
#define TWO (1<<1)
#define THERE (1<<2)
#define FOUR (1<<3)

void show(int flags)
{
    if(flags & ONE) printf("one\n");
    if(flags & TWO) printf("two\n");
    if(flags & THERE) printf("there\n");
    if(flags & FOUR) printf("four\n");
}

int main()
{
    umask(0);

    printf("stdin->fd:%d\n",stdin->_fileno);
    printf("stdout->fd:%d\n",stdout->_fileno);
    printf("stderr->fd:%d\n",stderr->_fileno);
    //打开
   //int fd=open("log.txt",O_WRONLY|O_CREAT|O_TRUNC,0666);
   // int fd=open("log.txt",O_RDONLY);
   int fd0=open(FILE_NAME(1),O_WRONLY|O_CREAT|O_TRUNC,0666);
   int fd1=open(FILE_NAME(2),O_WRONLY|O_CREAT|O_TRUNC,0666);
   int fd2=open(FILE_NAME(3),O_WRONLY|O_CREAT|O_TRUNC,0666);
   int fd3=open(FILE_NAME(4),O_WRONLY|O_CREAT|O_TRUNC,0666);
   int fd4=open(FILE_NAME(5),O_WRONLY|O_CREAT|O_TRUNC,0666);
  //  if(fd<0)
  //  {
  //      perror("open");
  //      return 1;
  //  }

    printf("fd0:%d\n",fd0);
    printf("fd1:%d\n",fd1);
    printf("fd2:%d\n",fd2);
    printf("fd3:%d\n",fd3);
    printf("fd4:%d\n",fd4);

    close(fd0);
    close(fd1);
    close(fd2);
    close(fd3);
    close(fd4);
    //读
  // char buffer[1024];
  // ssize_t num=read(fd,buffer,sizeof(buffer)-1);
  // if(num > 0) buffer[num-1]=0; //0,\0,NULL--->0,,系统调用接口不管读的是什么类型文件，都会当作二进制处理，这里我们想读的是字符串，因此结尾加个\0
  // puts(buffer);


    //写
   // const char* buffer="hello linux\n";
   // const char* buffer="aaaaaa\n";
   // ssize_t num= write(fd,buffer,strlen(buffer));
   // if(num<0)
   // {
   //     perror("write");
   //     exit(2);
   // }
    


    //关闭
   // close(fd);

 //   show(ONE);
 //   printf("------------------\n");
 //   show(ONE|TWO);
 //   printf("------------------\n");
 //   show(ONE|TWO|THERE);
 //   printf("------------------\n");
 //   show(ONE|TWO|THERE|FOUR);
 //   printf("------------------\n");
    

    //打开
 //   FILE* fp=fopen("log.txt","w");
 //  // FILE* fp=fopen("log.txt","r");
 //  // FILE* fp=fopen("log.txt","a");
 //   if(fp == NULL)
 //   {
 //       perror("fopen");
 //       return 1;
 //   }

 //  //写
 //  int cnt=5;
 //  while(cnt)
 //  {
 //      fprintf(fp,"%s:%d\n","hello linux",cnt--);
 //  }
   
  // //读
  //  char buffer[64];
  //  while(fgets(buffer,sizeof(buffer)-1,fp) !=  NULL)
  //  {
  //      buffer[strlen(buffer)-1]=0;
  //      puts(buffer);
  //  }
    
    //关闭
 //   fclose(fp);
    return 0;
}
