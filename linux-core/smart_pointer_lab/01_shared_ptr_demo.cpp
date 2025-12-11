#include <iostream>
#include <memory>
#include <string>
#include <vector>

// 示例类：资源对象
class Resource {
public:
    Resource(const std::string& name) : name_(name) {
        std::cout << "Resource [" << name_ << "] 构造函数调用" << std::endl;
    }
    
    ~Resource() {
        std::cout << "Resource [" << name_ << "] 析构函数调用" << std::endl;
    }
    
    void doWork() {
        std::cout << "Resource [" << name_ << "] 正在工作..." << std::endl;
    }
    
    const std::string& getName() const { return name_; }

private:
    std::string name_;
};

// 演示shared_ptr的基本使用和引用计数
void demonstrateBasicUsage() {
    std::cout << "\n=== shared_ptr 基本使用和引用计数演示 ===" << std::endl;
    
    // 创建shared_ptr
    std::shared_ptr<Resource> ptr1 = std::make_shared<Resource>("资源A");
    std::cout << "创建ptr1后，引用计数: " << ptr1.use_count() << std::endl;
    
    {
        // 复制shared_ptr，引用计数增加
        std::shared_ptr<Resource> ptr2 = ptr1;
        std::cout << "创建ptr2后，引用计数: " << ptr1.use_count() << std::endl;
        
        {
            // 再次复制，引用计数继续增加
            std::shared_ptr<Resource> ptr3 = ptr1;
            std::cout << "创建ptr3后，引用计数: " << ptr1.use_count() << std::endl;
            
            ptr3->doWork();
        } // ptr3离开作用域，引用计数减少
        
        std::cout << "ptr3离开作用域后，引用计数: " << ptr1.use_count() << std::endl;
    } // ptr2离开作用域，引用计数减少
    
    std::cout << "ptr2离开作用域后，引用计数: " << ptr1.use_count() << std::endl;
    
} // ptr1离开作用域，引用计数归零，对象被销毁

// 演示多对象共享资源的场景
class DataProcessor {
public:
    DataProcessor(std::shared_ptr<Resource> resource) : resource_(resource) {
        std::cout << "DataProcessor 创建，共享资源: " << resource_->getName() << std::endl;
    }
    
    void process() {
        if (resource_) {
            std::cout << "DataProcessor 使用共享资源进行处理" << std::endl;
            resource_->doWork();
        }
    }
    
private:
    std::shared_ptr<Resource> resource_;
};

class Logger {
public:
    Logger(std::shared_ptr<Resource> resource) : resource_(resource) {
        std::cout << "Logger 创建，共享资源: " << resource_->getName() << std::endl;
    }
    
    void log() {
        if (resource_) {
            std::cout << "Logger 使用共享资源进行日志记录" << std::endl;
            resource_->doWork();
        }
    }
    
private:
    std::shared_ptr<Resource> resource_;
};

void demonstrateSharedOwnership() {
    std::cout << "\n=== 多对象共享资源演示 ===" << std::endl;
    
    // 创建共享资源
    auto sharedResource = std::make_shared<Resource>("共享数据库连接");
    std::cout << "创建共享资源后，引用计数: " << sharedResource.use_count() << std::endl;
    
    // 多个对象共享同一资源
    {
        DataProcessor processor(sharedResource);
        std::cout << "创建DataProcessor后，引用计数: " << sharedResource.use_count() << std::endl;
        
        Logger logger(sharedResource);
        std::cout << "创建Logger后，引用计数: " << sharedResource.use_count() << std::endl;
        
        // 使用共享资源
        processor.process();
        logger.log();
        
        std::cout << "对象使用完毕，引用计数: " << sharedResource.use_count() << std::endl;
    } // processor和logger离开作用域
    
    std::cout << "processor和logger离开作用域后，引用计数: " << sharedResource.use_count() << std::endl;
} // sharedResource离开作用域，资源被自动释放

// 演示容器中的shared_ptr
void demonstrateContainerUsage() {
    std::cout << "\n=== 容器中的shared_ptr演示 ===" << std::endl;
    
    std::vector<std::shared_ptr<Resource>> resources;
    
    // 创建资源并添加到容器
    auto resource1 = std::make_shared<Resource>("资源1");
    auto resource2 = std::make_shared<Resource>("资源2");
    
    resources.push_back(resource1);
    resources.push_back(resource2);
    resources.push_back(resource1); // 同一资源被多次引用
    
    std::cout << "资源1的引用计数: " << resource1.use_count() << std::endl;
    std::cout << "资源2的引用计数: " << resource2.use_count() << std::endl;
    
    // 使用容器中的资源
    for (size_t i = 0; i < resources.size(); ++i) {
        std::cout << "使用容器中第" << i+1 << "个资源: ";
        resources[i]->doWork();
    }
    
    // 清空容器
    resources.clear();
    std::cout << "清空容器后，资源1的引用计数: " << resource1.use_count() << std::endl;
    std::cout << "清空容器后，资源2的引用计数: " << resource2.use_count() << std::endl;
}

// 演示reset和自定义删除器
void demonstrateResetAndDeleter() {
    std::cout << "\n=== reset和自定义删除器演示 ===" << std::endl;
    
    // 自定义删除器
    auto customDeleter = [](Resource* ptr) {
        std::cout << "自定义删除器被调用，删除资源: " << ptr->getName() << std::endl;
        delete ptr;
    };
    
    std::shared_ptr<Resource> ptr(new Resource("自定义删除器资源"), customDeleter);
    std::cout << "创建带自定义删除器的shared_ptr，引用计数: " << ptr.use_count() << std::endl;
    
    auto ptr2 = ptr;
    std::cout << "复制后，引用计数: " << ptr.use_count() << std::endl;
    
    // 重置指针
    ptr.reset();
    std::cout << "ptr重置后，ptr2的引用计数: " << ptr2.use_count() << std::endl;
    
    // ptr2也重置，触发自定义删除器
    ptr2.reset();
    std::cout << "所有指针都重置完毕" << std::endl;
}

int main() {
    std::cout << "C++ 智能指针实验 - shared_ptr 演示" << std::endl;
    std::cout << "====================================" << std::endl;
    
    demonstrateBasicUsage();
    demonstrateSharedOwnership();
    demonstrateContainerUsage();
    demonstrateResetAndDeleter();
    
    std::cout << "\n程序结束" << std::endl;
    return 0;
}
