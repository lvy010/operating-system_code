#include<iostream>
#include<algorithm>
#include<vector>
using namespace std;


int main()
{
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);

    //find是由库里面提供的，直接使用
	//Vector<int>::iterator it = find(v.begin(), v.end(), 3);
	vector<int>::iterator it = find(v.begin(), v.end(), 3);
	if (it != v.end())
	{
		//v.insert(it, 10);
		v.erase(it);
	}

	//使用STL，vector删除it位置，it位置失效了，不能在对it位置读写，访问。

	(*it)++;

    vector<int>::iterator it1=v.begin();
    while(it1 != v.end())
    {
        cout<<*it1<<" ";
        ++it1;
    }
    cout<<endl;

}
