#include<stdio.h>

int Addtoval(int begin,int end)
{
    int sum=0;
    for(begin;begin<=end;++begin)
    {
        sum+=begin;
    }

    return sum;
}

void Print(int sum)
{
    printf("sum=%d\n",sum);
}

int main()
{
    int sum= Addtoval(0,100);
    Print(sum);
    return 0;
}
