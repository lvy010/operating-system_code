#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>

int main()
{
    printf("hello printf\n");
    fprintf(stdout,"%s\n","hello fprintf");


    const char* output="hello write\n";
    write(1,output,strlen(output));

    fork();
    return 0;
}
