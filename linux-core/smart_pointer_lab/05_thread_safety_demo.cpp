#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <random>

// 线程安全的计数器类
class Counter {
public:
    Counter(const std::string& name) : name_(name), count_(0) {
        std::cout << "Counter [" << name_ << "] 构造函数调用" << std::endl;
    }
    
    ~Counter() {
        std::cout << "Counter [" << name_ << "] 析构函数调用，最终计数: " << count_ << std::endl;
    }
    
    void increment() {
        std::lock_guard<std::mutex> lock(mutex_);
        ++count_;
        // 模拟一些工作
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
    
    void safeIncrement() {
        std::lock_guard<std::mutex> lock(mutex_);
        ++count_;
        std::cout << "线程 " << std::this_thread::get_id() 
                  << " 安全递增计数器 [" << name_ << "] 到 " << count_ << std::endl;
    }
    
    int getCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return count_;
    }
    
    const std::string& getName() const { return name_; }

private:
    std::string name_;
    int count_;
    mutable std::mutex mutex_;
};

// 演示shared_ptr引用计数的线程安全性
void demonstrateRefCountThreadSafety() {
    std::cout << "\n=== shared_ptr 引用计数线程安全性演示 ===" << std::endl;
    
    auto counter = std::make_shared<Counter>("线程安全计数器");
    std::cout << "初始引用计数: " << counter.use_count() << std::endl;
    
    const int numThreads = 4;
    const int operationsPerThread = 1000;
    
    std::vector<std::thread> threads;
    std::atomic<int> totalCopies(0);
    
    // 启动多个线程，每个线程都复制和释放shared_ptr
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&counter, &totalCopies, operationsPerThread, i]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(1, 10);
            
            for (int j = 0; j < operationsPerThread; ++j) {
                // 创建shared_ptr的副本
                auto localCopy = counter;
                totalCopies.fetch_add(1);
                
                // 模拟一些工作
                localCopy->increment();
                
                // 随机睡眠一小段时间
                std::this_thread::sleep_for(std::chrono::microseconds(dis(gen)));
                
                // localCopy离开作用域时会自动减少引用计数
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "所有线程完成后:" << std::endl;
    std::cout << "  引用计数: " << counter.use_count() << std::endl;
    std::cout << "  计数器值: " << counter->getCount() << std::endl;
    std::cout << "  总副本数: " << totalCopies.load() << std::endl;
    std::cout << "  期望的计数器值: " << numThreads * operationsPerThread << std::endl;
}

// 演示对象访问需要额外同步
void demonstrateObjectAccessSafety() {
    std::cout << "\n=== 对象访问线程安全演示 ===" << std::endl;
    
    auto counter = std::make_shared<Counter>("访问安全测试");
    const int numThreads = 3;
    const int operationsPerThread = 5;
    
    std::vector<std::thread> threads;
    
    // 启动多个线程访问同一个对象
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([counter, operationsPerThread, i]() {
            for (int j = 0; j < operationsPerThread; ++j) {
                // shared_ptr的复制是线程安全的
                auto localPtr = counter;
                
                // 但对象的方法调用需要对象自身提供线程安全保证
                localPtr->safeIncrement();
                
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "最终计数: " << counter->getCount() << std::endl;
}

// 演示shared_ptr在不同线程间传递
class Task {
public:
    Task(int id, std::shared_ptr<Counter> counter) 
        : id_(id), counter_(counter) {
        std::cout << "Task " << id_ << " 创建，持有计数器引用" << std::endl;
    }
    
    ~Task() {
        std::cout << "Task " << id_ << " 销毁" << std::endl;
    }
    
    void execute() {
        std::cout << "Task " << id_ << " 开始执行，线程: " 
                  << std::this_thread::get_id() << std::endl;
        
        for (int i = 0; i < 3; ++i) {
            counter_->increment();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        std::cout << "Task " << id_ << " 执行完成" << std::endl;
    }

private:
    int id_;
    std::shared_ptr<Counter> counter_;
};

void demonstrateSharedPtrPassing() {
    std::cout << "\n=== shared_ptr 跨线程传递演示 ===" << std::endl;
    
    auto counter = std::make_shared<Counter>("跨线程计数器");
    std::cout << "创建计数器后引用计数: " << counter.use_count() << std::endl;
    
    std::vector<std::thread> threads;
    std::vector<std::unique_ptr<Task>> tasks;
    
    // 创建任务对象，每个都持有shared_ptr
    for (int i = 0; i < 3; ++i) {
        tasks.push_back(std::make_unique<Task>(i + 1, counter));
    }
    
    std::cout << "创建任务后引用计数: " << counter.use_count() << std::endl;
    
    // 在不同线程中执行任务
    for (auto& task : tasks) {
        threads.emplace_back([&task]() {
            task->execute();
        });
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "任务执行完成后引用计数: " << counter.use_count() << std::endl;
    
    // 清理任务
    tasks.clear();
    std::cout << "清理任务后引用计数: " << counter.use_count() << std::endl;
}

// 演示竞态条件和解决方案
class UnsafeResource {
public:
    UnsafeResource() : value_(0) {}
    
    void unsafeIncrement() {
        // 这里故意不加锁，演示竞态条件
        int temp = value_;
        std::this_thread::sleep_for(std::chrono::microseconds(1)); // 模拟延迟
        value_ = temp + 1;
    }
    
    void safeIncrement() {
        std::lock_guard<std::mutex> lock(mutex_);
        int temp = value_;
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        value_ = temp + 1;
    }
    
    int getValue() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return value_;
    }

private:
    int value_;
    mutable std::mutex mutex_;
};

void demonstrateRaceCondition() {
    std::cout << "\n=== 竞态条件演示 ===" << std::endl;
    
    // 不安全的版本
    {
        std::cout << "--- 不安全的版本（存在竞态条件） ---" << std::endl;
        auto resource = std::make_shared<UnsafeResource>();
        const int numThreads = 4;
        const int incrementsPerThread = 100;
        
        std::vector<std::thread> threads;
        
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back([resource, incrementsPerThread]() {
                for (int j = 0; j < incrementsPerThread; ++j) {
                    resource->unsafeIncrement();
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        std::cout << "期望值: " << numThreads * incrementsPerThread << std::endl;
        std::cout << "实际值: " << resource->getValue() << std::endl;
        std::cout << "差异: " << (numThreads * incrementsPerThread - resource->getValue()) << std::endl;
    }
    
    // 安全的版本
    {
        std::cout << "\n--- 安全的版本（使用互斥锁） ---" << std::endl;
        auto resource = std::make_shared<UnsafeResource>();
        const int numThreads = 4;
        const int incrementsPerThread = 100;
        
        std::vector<std::thread> threads;
        
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back([resource, incrementsPerThread]() {
                for (int j = 0; j < incrementsPerThread; ++j) {
                    resource->safeIncrement();
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        std::cout << "期望值: " << numThreads * incrementsPerThread << std::endl;
        std::cout << "实际值: " << resource->getValue() << std::endl;
        std::cout << "差异: " << (numThreads * incrementsPerThread - resource->getValue()) << std::endl;
    }
}

// 演示shared_ptr的原子操作
void demonstrateAtomicSharedPtr() {
    std::cout << "\n=== shared_ptr 原子操作演示 ===" << std::endl;
    
    std::shared_ptr<Counter> globalPtr = std::make_shared<Counter>("原子测试计数器");
    std::mutex printMutex; // 用于同步输出
    
    const int numThreads = 3;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&globalPtr, &printMutex, i]() {
            for (int j = 0; j < 3; ++j) {
                // 原子地读取shared_ptr
                auto localPtr = std::atomic_load(&globalPtr);
                
                {
                    std::lock_guard<std::mutex> lock(printMutex);
                    std::cout << "线程 " << i << " 读取到计数器: " 
                              << localPtr->getName() << ", 引用计数: " 
                              << localPtr.use_count() << std::endl;
                }
                
                localPtr->increment();
                
                // 创建新的计数器并原子地更新
                if (j == 1) {
                    auto newPtr = std::make_shared<Counter>("新计数器" + std::to_string(i));
                    std::atomic_store(&globalPtr, newPtr);
                    
                    std::lock_guard<std::mutex> lock(printMutex);
                    std::cout << "线程 " << i << " 更新了全局指针" << std::endl;
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "最终全局指针指向: " << globalPtr->getName() << std::endl;
}

int main() {
    std::cout << "C++ 智能指针实验 - shared_ptr 线程安全性" << std::endl;
    std::cout << "=========================================" << std::endl;
    
    demonstrateRefCountThreadSafety();
    demonstrateObjectAccessSafety();
    demonstrateSharedPtrPassing();
    demonstrateRaceCondition();
    demonstrateAtomicSharedPtr();
    
    std::cout << "\n程序结束" << std::endl;
    return 0;
}
