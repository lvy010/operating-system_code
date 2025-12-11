#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <thread>

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

// 演示weak_ptr的基本操作
void demonstrateBasicWeakPtrOperations() {
    std::cout << "\n=== weak_ptr 基本操作演示 ===" << std::endl;
    
    std::weak_ptr<Resource> weakPtr;
    
    {
        auto sharedPtr = std::make_shared<Resource>("测试资源");
        std::cout << "创建shared_ptr后，引用计数: " << sharedPtr.use_count() << std::endl;
        
        // 从shared_ptr创建weak_ptr
        weakPtr = sharedPtr;
        std::cout << "创建weak_ptr后，shared_ptr引用计数: " << sharedPtr.use_count() << std::endl;
        std::cout << "weak_ptr引用计数: " << weakPtr.use_count() << std::endl;
        
        // 检查weak_ptr是否过期
        if (!weakPtr.expired()) {
            std::cout << "weak_ptr未过期，可以安全访问" << std::endl;
            
            // 通过lock()获取shared_ptr
            if (auto lockedPtr = weakPtr.lock()) {
                std::cout << "通过lock()获得临时shared_ptr，引用计数: " << lockedPtr.use_count() << std::endl;
                lockedPtr->doWork();
            }
        }
        
        std::cout << "shared_ptr即将离开作用域..." << std::endl;
    } // shared_ptr离开作用域，资源被销毁
    
    // 检查weak_ptr是否过期
    std::cout << "shared_ptr已离开作用域" << std::endl;
    if (weakPtr.expired()) {
        std::cout << "weak_ptr已过期，指向的对象已被销毁" << std::endl;
    }
    
    // 尝试lock已过期的weak_ptr
    if (auto lockedPtr = weakPtr.lock()) {
        std::cout << "意外：lock()成功了！" << std::endl;
    } else {
        std::cout << "lock()返回空指针，确认对象已被销毁" << std::endl;
    }
}

// 缓存管理器：使用weak_ptr实现临时缓存
class CacheManager {
public:
    std::shared_ptr<Resource> getResource(const std::string& name) {
        // 首先检查缓存中是否存在
        auto it = cache_.find(name);
        if (it != cache_.end()) {
            // 尝试从weak_ptr获取shared_ptr
            if (auto resource = it->second.lock()) {
                std::cout << "从缓存中获取资源: " << name << std::endl;
                return resource;
            } else {
                // 资源已被销毁，从缓存中移除
                std::cout << "缓存中的资源已过期，移除: " << name << std::endl;
                cache_.erase(it);
            }
        }
        
        // 创建新资源并添加到缓存
        std::cout << "创建新资源并添加到缓存: " << name << std::endl;
        auto resource = std::make_shared<Resource>(name);
        cache_[name] = resource; // weak_ptr不会影响引用计数
        return resource;
    }
    
    void showCacheStatus() {
        std::cout << "缓存状态:" << std::endl;
        auto it = cache_.begin();
        while (it != cache_.end()) {
            if (it->second.expired()) {
                std::cout << "  [" << it->first << "] - 已过期，即将移除" << std::endl;
                it = cache_.erase(it);
            } else {
                std::cout << "  [" << it->first << "] - 有效" << std::endl;
                ++it;
            }
        }
        std::cout << "缓存中有效条目数: " << cache_.size() << std::endl;
    }
    
    void clearExpiredEntries() {
        auto it = cache_.begin();
        while (it != cache_.end()) {
            if (it->second.expired()) {
                std::cout << "清理过期缓存条目: " << it->first << std::endl;
                it = cache_.erase(it);
            } else {
                ++it;
            }
        }
    }

private:
    std::map<std::string, std::weak_ptr<Resource>> cache_;
};

void demonstrateCacheManager() {
    std::cout << "\n=== 缓存管理器演示 ===" << std::endl;
    
    CacheManager cacheManager;
    
    {
        // 第一次获取资源
        auto resource1 = cacheManager.getResource("数据库连接");
        auto resource2 = cacheManager.getResource("文件句柄");
        
        cacheManager.showCacheStatus();
        
        // 再次获取相同资源（应该从缓存获取）
        auto resource3 = cacheManager.getResource("数据库连接");
        
        std::cout << "resource1和resource3是否指向同一对象: " 
                  << (resource1.get() == resource3.get() ? "是" : "否") << std::endl;
        
        cacheManager.showCacheStatus();
        
        // 释放一个资源
        resource2.reset();
        std::cout << "释放resource2后:" << std::endl;
        cacheManager.showCacheStatus();
        
    } // 所有shared_ptr离开作用域
    
    std::cout << "所有shared_ptr离开作用域后:" << std::endl;
    cacheManager.showCacheStatus();
    
    // 清理过期条目
    cacheManager.clearExpiredEntries();
    cacheManager.showCacheStatus();
}

// 回调管理器：使用weak_ptr避免悬空指针
class EventSource;

class EventListener {
public:
    EventListener(const std::string& name) : name_(name) {
        std::cout << "EventListener [" << name_ << "] 构造函数调用" << std::endl;
    }
    
    ~EventListener() {
        std::cout << "EventListener [" << name_ << "] 析构函数调用" << std::endl;
    }
    
    void onEvent(const std::string& event) {
        std::cout << "EventListener [" << name_ << "] 收到事件: " << event << std::endl;
    }
    
    const std::string& getName() const { return name_; }

private:
    std::string name_;
};

class EventSource {
public:
    void addListener(std::shared_ptr<EventListener> listener) {
        listeners_.push_back(listener); // 存储weak_ptr
        std::cout << "添加事件监听器: " << listener->getName() << std::endl;
    }
    
    void fireEvent(const std::string& event) {
        std::cout << "触发事件: " << event << std::endl;
        
        // 遍历监听器，自动清理无效的weak_ptr
        auto it = listeners_.begin();
        while (it != listeners_.end()) {
            if (auto listener = it->lock()) {
                listener->onEvent(event);
                ++it;
            } else {
                std::cout << "  移除已销毁的监听器" << std::endl;
                it = listeners_.erase(it);
            }
        }
    }
    
    void showListenerCount() {
        // 清理过期的监听器
        auto it = listeners_.begin();
        while (it != listeners_.end()) {
            if (it->expired()) {
                it = listeners_.erase(it);
            } else {
                ++it;
            }
        }
        
        std::cout << "当前有效监听器数量: " << listeners_.size() << std::endl;
    }

private:
    std::vector<std::weak_ptr<EventListener>> listeners_;
};

void demonstrateEventSystem() {
    std::cout << "\n=== 事件系统演示 ===" << std::endl;
    
    EventSource eventSource;
    
    {
        auto listener1 = std::make_shared<EventListener>("监听器1");
        auto listener2 = std::make_shared<EventListener>("监听器2");
        
        eventSource.addListener(listener1);
        eventSource.addListener(listener2);
        
        eventSource.showListenerCount();
        eventSource.fireEvent("初始事件");
        
        // 销毁一个监听器
        listener1.reset();
        std::cout << "\n销毁监听器1后:" << std::endl;
        eventSource.fireEvent("第二个事件");
        
    } // listener2也离开作用域
    
    std::cout << "\n所有监听器都离开作用域后:" << std::endl;
    eventSource.showListenerCount();
    eventSource.fireEvent("最后一个事件");
}

// 父子关系管理：避免环形引用的同时保持关系
class TreeNode {
public:
    TreeNode(const std::string& name) : name_(name) {
        std::cout << "TreeNode [" << name_ << "] 构造函数调用" << std::endl;
    }
    
    ~TreeNode() {
        std::cout << "TreeNode [" << name_ << "] 析构函数调用" << std::endl;
    }
    
    void addChild(std::shared_ptr<TreeNode> child) {
        children_.push_back(child);
        child->parent_ = shared_from_this(); // weak_ptr不增加引用计数
        std::cout << "TreeNode [" << name_ << "] 添加子节点: " << child->getName() << std::endl;
    }
    
    void showFamily() {
        std::cout << "TreeNode [" << name_ << "]:" << std::endl;
        
        // 显示父节点
        if (auto parent = parent_.lock()) {
            std::cout << "  父节点: " << parent->getName() << std::endl;
        } else {
            std::cout << "  父节点: 无" << std::endl;
        }
        
        // 显示子节点
        std::cout << "  子节点数: " << children_.size() << std::endl;
        for (size_t i = 0; i < children_.size(); ++i) {
            if (children_[i]) {
                std::cout << "    " << (i+1) << ". " << children_[i]->getName() << std::endl;
            }
        }
    }
    
    std::shared_ptr<TreeNode> getParent() {
        return parent_.lock(); // 安全地获取父节点
    }
    
    const std::string& getName() const { return name_; }

private:
    std::string name_;
    std::weak_ptr<TreeNode> parent_;              // 弱引用父节点
    std::vector<std::shared_ptr<TreeNode>> children_; // 强引用子节点
};

// 需要继承enable_shared_from_this
class SafeTreeNode : public std::enable_shared_from_this<SafeTreeNode> {
public:
    SafeTreeNode(const std::string& name) : name_(name) {
        std::cout << "SafeTreeNode [" << name_ << "] 构造函数调用" << std::endl;
    }
    
    ~SafeTreeNode() {
        std::cout << "SafeTreeNode [" << name_ << "] 析构函数调用" << std::endl;
    }
    
    void addChild(std::shared_ptr<SafeTreeNode> child) {
        children_.push_back(child);
        child->parent_ = shared_from_this();
        std::cout << "SafeTreeNode [" << name_ << "] 添加子节点: " << child->getName() << std::endl;
    }
    
    void showFamily() {
        std::cout << "SafeTreeNode [" << name_ << "]:" << std::endl;
        
        if (auto parent = parent_.lock()) {
            std::cout << "  父节点: " << parent->getName() << std::endl;
        } else {
            std::cout << "  父节点: 无" << std::endl;
        }
        
        std::cout << "  子节点数: " << children_.size() << std::endl;
        for (size_t i = 0; i < children_.size(); ++i) {
            if (children_[i]) {
                std::cout << "    " << (i+1) << ". " << children_[i]->getName() << std::endl;
            }
        }
    }
    
    const std::string& getName() const { return name_; }

private:
    std::string name_;
    std::weak_ptr<SafeTreeNode> parent_;
    std::vector<std::shared_ptr<SafeTreeNode>> children_;
};

void demonstrateTreeStructure() {
    std::cout << "\n=== 树形结构演示 ===" << std::endl;
    
    {
        auto root = std::make_shared<SafeTreeNode>("根节点");
        auto child1 = std::make_shared<SafeTreeNode>("子节点1");
        auto child2 = std::make_shared<SafeTreeNode>("子节点2");
        auto grandchild = std::make_shared<SafeTreeNode>("孙节点");
        
        // 构建树形结构
        root->addChild(child1);
        root->addChild(child2);
        child1->addChild(grandchild);
        
        // 显示家族关系
        root->showFamily();
        child1->showFamily();
        grandchild->showFamily();
        
        std::cout << "\n即将离开作用域..." << std::endl;
    }
    
    std::cout << "树形结构演示完成" << std::endl;
}

// 演示weak_ptr的计数和空间管理
void demonstrateWeakPtrCounting() {
    std::cout << "\n=== weak_ptr 计数和空间管理演示 ===" << std::endl;
    
    std::weak_ptr<Resource> weak1, weak2, weak3;
    
    {
        auto shared = std::make_shared<Resource>("计数测试资源");
        
        std::cout << "创建shared_ptr后:" << std::endl;
        std::cout << "  shared_ptr引用计数: " << shared.use_count() << std::endl;
        
        // 创建多个weak_ptr
        weak1 = shared;
        weak2 = shared;
        weak3 = weak1; // 从另一个weak_ptr复制
        
        std::cout << "创建3个weak_ptr后:" << std::endl;
        std::cout << "  shared_ptr引用计数: " << shared.use_count() << std::endl;
        std::cout << "  weak_ptr引用计数: " << weak1.use_count() << std::endl;
        
        // weak_ptr不影响对象生命周期
        std::cout << "  weak1.expired(): " << (weak1.expired() ? "是" : "否") << std::endl;
        std::cout << "  weak2.expired(): " << (weak2.expired() ? "是" : "否") << std::endl;
        std::cout << "  weak3.expired(): " << (weak3.expired() ? "是" : "否") << std::endl;
        
        std::cout << "\nshared_ptr即将离开作用域..." << std::endl;
    } // shared_ptr离开作用域，对象被销毁，但控制块可能仍存在
    
    std::cout << "\nshared_ptr离开作用域后:" << std::endl;
    std::cout << "  weak1.expired(): " << (weak1.expired() ? "是" : "否") << std::endl;
    std::cout << "  weak2.expired(): " << (weak2.expired() ? "是" : "否") << std::endl;
    std::cout << "  weak3.expired(): " << (weak3.expired() ? "是" : "否") << std::endl;
    
    // 重置weak_ptr
    weak1.reset();
    weak2.reset();
    // weak3会在离开作用域时自动重置
    
    std::cout << "重置weak_ptr后，控制块也会被释放" << std::endl;
}

int main() {
    std::cout << "C++ 智能指针实验 - weak_ptr 其他使用场景" << std::endl;
    std::cout << "===========================================" << std::endl;
    
    demonstrateBasicWeakPtrOperations();
    demonstrateCacheManager();
    demonstrateEventSystem();
    demonstrateTreeStructure();
    demonstrateWeakPtrCounting();
    
    std::cout << "\n程序结束" << std::endl;
    return 0;
}
