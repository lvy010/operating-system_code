#include<stdio.h>


int main()
{
   // FILE* fp=fopen("bite.txt","w");
    FILE* fp=fopen("bite.txt","r");
    if(fp == NULL)
    {
        perror("fopen");
        return 1;
    }

    //写
   // fprintf(fp,"%s\n","linux so esay!");
    //读
    char buff[64];
    fgets(buff,sizeof(buff),fp);
    printf("%s",buff);

    fclose(fp);

    return 0;

}
