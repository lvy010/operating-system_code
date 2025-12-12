#include"my_add.h"
#include"my_sub.h"

int main()
{

    int a=10,b=20;
    int ret=Add(a,b);
    printf("result:%d\n",ret);
    ret=Sub(a,b);
    printf("result:%d\n",ret);


    return 0;
}
