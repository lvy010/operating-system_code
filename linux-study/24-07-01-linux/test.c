#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#define USER "USER"

//int main()
//{
//    //getenv获取环境变量
//   // char* user=getenv(USER);
//   // if(user)
//   // {
//   //     printf("%s\n",user);
//   // }
//   // else
//   // {
//   //     printf("%s no found\n",user);
//   // }
//    
//    //本地变量
//    char* a=getenv("a");
//    if(a)
//    {
//        printf("%s\n",a);
//    }
//    else
//    {
//        printf("%s no found\n",a);
//    }
//   
//   
//        
//    
//    return 0;
//}

//ls -a -l -d 是一个长长的字符串
//"ls" "-a" "-l" "-d"  以空格为分隔符，分成一个个字符串
//int main(int argc,char* argv[],char* env[])
int main()
{
    extern char** environ;

    for(int i=0;environ[i];++i)
    {
        printf("%d : %s \n",i,environ[i]);
    }







    //for(int i=0;env[i];++i)
    //{
    //    printf("env[%d]:%s\n",i,env[i]);
    //}
    



   // for(int i=0;i<argc;++i)
   // {
   //     printf("argv[%d]----->%s\n",i,argv[i]);
   // }
    //以./mytest -a -l -d为例
  //  if(strcmp("-a",argv[1]) == 0)
  //  {
  //      printf("功能a\n");
  //  }

  //  if(strcmp("-l",argv[2]) == 0)
  //  {
  //      printf("功能l\n");
  //  }

  //  if(strcmp("-d",argv[3]) == 0)
  //  {
  //       printf("功能l\n");
  //  }
    
  // if(argc != 2)
  // {
  //     printf("Usage: \n\t%s [-a/-b/-c/-ab/-bc/-ac/-abc]\n", argv[0]);
  //     return 1;
  // }
  // if(strcmp("-a", argv[1]) == 0)
  // {
  //     printf("功能a\n");
  // }
  // if(strcmp("-b", argv[1]) == 0)
  // {
  //     printf("功能b\n");
  // }
  // if(strcmp("-c", argv[1]) == 0)
  // {
  //     printf("功能c\n");
  // }
  // if(strcmp("-ab", argv[1]) == 0)
  // {
  //     printf("功能ab\n");
  // }
  // if(strcmp("-bc", argv[1]) == 0)
  // {
  //     printf("功能bc\n");
  // }
    return 0;
}
