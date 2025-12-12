#include"process.h"


void Processon()
{
    char style[]={'#','>','.','~'};

    int i=0;
    char bar[NUM];
    const char* lable="|\\-/";
    memset(bar,'\0',sizeof(bar));
    while(i<=100)
    {
        printf("[%-100s][%d%%][%c]\r",bar,i,lable[i%4]);
        fflush(stdout);
        bar[i++]=style[N];
        usleep(50000);
    }

    printf("\n");
}
