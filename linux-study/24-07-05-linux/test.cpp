#include<iostream>
#include<vector>
#include<algorithm>
using namespace std;


void test_Vector2()
{
	//Vector<int> v;
	vector<int> v;
	v.push_back(1);
	v.push_back(2);
	v.push_back(3);
	v.push_back(4);


	//v.insert(v.begin() + 1, 10);
	//find由算法库提供
	//这是自己实现的Vector
	//Vector<int>::iterator it = find(v.begin(), v.end(), 3);
	//官方库实现的看一看
	std::vector<int>::iterator it = find(v.begin(), v.end(), 4);
	if (it != v.end())
	{
		v.erase(it);
	}

	cout << *it << endl;


}

int main()
{
   test_Vector2(); 
    return 0;
}
