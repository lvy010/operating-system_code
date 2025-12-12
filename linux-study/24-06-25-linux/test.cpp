#include<iostream>
#include<string>
#include<list>
#include<stdio.h>
using namespace std;

int main()
{
    string s1("1111");
   // string s2("22222222222222222222222222222222");
    string s2(s1);
    cout<<sizeof(s1)<<endl;
    cout<<sizeof(s2)<<endl;
    
    printf("%p\n",s1.c_str());
    printf("%p\n",s2.c_str());
    
    s2[0]++;
    printf("%p\n",s1.c_str());
    printf("%p\n",s2.c_str());
   // list<int> it;
   // it.push_back(1);
   // it.push_back(2);
   // it.push_back(3);
   // it.push_back(4);

   // list<int>::iterator it1=it.begin();
   // while(it1 != it.end())
   // {
   //     cout<<*it1<<" ";
   //     ++it1;
   // }
   // cout<<endl;
    return 0;
}
