#include <memory>
#include <iostream>

class MyClass 
{
public:
    void DoSomething() {
        std::cout << "MyClass::DoSomething() called" << std::endl;
    }
};

int main() 
{
    // 创建一个 shared_ptr 管理对象
    std::shared_ptr<MyClass> sharedObj = std::make_shared<MyClass>();

    // 通过 shared_ptr 创建 weak_ptr
    std::weak_ptr<MyClass> weakObj = sharedObj;

    // 访问 weak_ptr 指向的对象
    if (auto tempShared = weakObj.lock()) 
    {
        // 对象存活，tempShared 是有效的 shared_ptr
        tempShared->DoSomething();
    } 
    else 
    {
        std::cout << "对象已被释放" << std::endl;
    }

    // 释放 shared_ptr，对象被销毁
    sharedObj.reset();

    // 再次尝试访问
    if (auto tempShared = weakObj.lock()) 
    {
        tempShared->DoSomething();
    } 
    else {
        std::cout << "对象已被释放" << std::endl; // 输出此分支
    }
} 