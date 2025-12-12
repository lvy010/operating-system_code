#include<iostream>
#include<string>
using namespace std;

int main()
{
    string s;
    size_t capacity=s.capacity();
    cout<<"capacity:"<<capacity<<endl;
    for(int i=0;i<1000;++i)
    {
        s.push_back('x');
        if(capacity != s.capacity())
        {
            capacity=s.capacity();
            cout<<"capacity changed:"<<capacity<<endl;
        }
    }
    return 0;
}
