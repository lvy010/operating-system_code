#include <iostream>
#include <memory>
#include <string>
#include <vector>

// 前向声明
class Child;

// 父节点类
class Parent {
public:
    Parent(const std::string& name) : name_(name) {
        std::cout << "Parent [" << name_ << "] 构造函数调用" << std::endl;
    }
    
    ~Parent() {
        std::cout << "Parent [" << name_ << "] 析构函数调用" << std::endl;
    }
    
    void addChild(std::shared_ptr<Child> child) {
        children_.push_back(child);
        std::cout << "Parent [" << name_ << "] 添加了一个子节点" << std::endl;
    }
    
    void showInfo() {
        std::cout << "Parent [" << name_ << "] 有 " << children_.size() << " 个子节点" << std::endl;
    }
    
    const std::string& getName() const { return name_; }

private:
    std::string name_;
    std::vector<std::shared_ptr<Child>> children_;
};

// 子节点类 - 有问题的版本（会造成环形引用）
class Child {
public:
    Child(const std::string& name) : name_(name) {
        std::cout << "Child [" << name_ << "] 构造函数调用" << std::endl;
    }
    
    ~Child() {
        std::cout << "Child [" << name_ << "] 析构函数调用" << std::endl;
    }
    
    void setParent(std::shared_ptr<Parent> parent) {
        parent_ = parent;
        std::cout << "Child [" << name_ << "] 设置了父节点: " << parent->getName() << std::endl;
    }
    
    void showInfo() {
        if (auto p = parent_.lock()) { // 如果是weak_ptr，使用lock()
            std::cout << "Child [" << name_ << "] 的父节点是: " << p->getName() << std::endl;
        } else if (parent_strong_) { // 如果是shared_ptr
            std::cout << "Child [" << name_ << "] 的父节点是: " << parent_strong_->getName() << std::endl;
        } else {
            std::cout << "Child [" << name_ << "] 没有父节点" << std::endl;
        }
    }
    
    const std::string& getName() const { return name_; }
    
    // 设置强引用父节点（会造成环形引用）
    void setParentStrong(std::shared_ptr<Parent> parent) {
        parent_strong_ = parent;
        std::cout << "Child [" << name_ << "] 设置了强引用父节点: " << parent->getName() << std::endl;
    }

private:
    std::string name_;
    std::weak_ptr<Parent> parent_;        // 弱引用，不会造成环形引用
    std::shared_ptr<Parent> parent_strong_; // 强引用，会造成环形引用
};

// 演示环形引用问题
void demonstrateCircularReferenceProblem() {
    std::cout << "\n=== 环形引用问题演示 ===" << std::endl;
    
    {
        auto parent = std::make_shared<Parent>("父节点1");
        auto child = std::make_shared<Child>("子节点1");
        
        std::cout << "创建对象后 - Parent引用计数: " << parent.use_count() 
                  << ", Child引用计数: " << child.use_count() << std::endl;
        
        // 建立环形引用
        parent->addChild(child);
        child->setParentStrong(parent); // 使用强引用，造成环形引用
        
        std::cout << "建立环形引用后 - Parent引用计数: " << parent.use_count() 
                  << ", Child引用计数: " << child.use_count() << std::endl;
        
        parent->showInfo();
        child->showInfo();
        
        std::cout << "即将离开作用域..." << std::endl;
    } // 离开作用域，但由于环形引用，对象不会被销毁！
    
    std::cout << "已离开作用域，检查对象是否被销毁..." << std::endl;
    std::cout << "注意：如果没有看到析构函数调用，说明发生了内存泄漏！" << std::endl;
}

// 演示使用weak_ptr解决环形引用
void demonstrateWeakPtrSolution() {
    std::cout << "\n=== 使用weak_ptr解决环形引用 ===" << std::endl;
    
    {
        auto parent = std::make_shared<Parent>("父节点2");
        auto child = std::make_shared<Child>("子节点2");
        
        std::cout << "创建对象后 - Parent引用计数: " << parent.use_count() 
                  << ", Child引用计数: " << child.use_count() << std::endl;
        
        // 建立正确的引用关系
        parent->addChild(child);
        child->setParent(parent); // 使用weak_ptr，不会造成环形引用
        
        std::cout << "建立引用关系后 - Parent引用计数: " << parent.use_count() 
                  << ", Child引用计数: " << child.use_count() << std::endl;
        
        parent->showInfo();
        child->showInfo();
        
        std::cout << "即将离开作用域..." << std::endl;
    } // 离开作用域，对象会被正确销毁
    
    std::cout << "已离开作用域，对象应该已被正确销毁" << std::endl;
}

// 更复杂的环形引用示例：双向链表节点
class ListNode {
public:
    ListNode(int value) : value_(value) {
        std::cout << "ListNode [" << value_ << "] 构造函数调用" << std::endl;
    }
    
    ~ListNode() {
        std::cout << "ListNode [" << value_ << "] 析构函数调用" << std::endl;
    }
    
    void setNext(std::shared_ptr<ListNode> next) {
        next_ = next;
    }
    
    void setPrev(std::shared_ptr<ListNode> prev) {
        prev_ = prev;
    }
    
    // 使用weak_ptr版本
    void setPrevWeak(std::shared_ptr<ListNode> prev) {
        prev_weak_ = prev;
    }
    
    void showConnections() {
        std::cout << "Node [" << value_ << "]: ";
        
        if (next_) {
            std::cout << "next=" << next_->value_ << " ";
        } else {
            std::cout << "next=null ";
        }
        
        if (prev_) {
            std::cout << "prev=" << prev_->value_ << " (强引用)";
        } else if (auto p = prev_weak_.lock()) {
            std::cout << "prev=" << p->value_ << " (弱引用)";
        } else {
            std::cout << "prev=null";
        }
        
        std::cout << std::endl;
    }
    
    int getValue() const { return value_; }

private:
    int value_;
    std::shared_ptr<ListNode> next_;
    std::shared_ptr<ListNode> prev_;      // 强引用，会造成环形引用
    std::weak_ptr<ListNode> prev_weak_;   // 弱引用，不会造成环形引用
};

void demonstrateLinkedListCircularReference() {
    std::cout << "\n=== 双向链表环形引用演示 ===" << std::endl;
    
    std::cout << "--- 有问题的版本（使用双向强引用） ---" << std::endl;
    {
        auto node1 = std::make_shared<ListNode>(1);
        auto node2 = std::make_shared<ListNode>(2);
        auto node3 = std::make_shared<ListNode>(3);
        
        // 建立双向强引用链
        node1->setNext(node2);
        node2->setPrev(node1);  // 强引用，造成环形引用
        node2->setNext(node3);
        node3->setPrev(node2);  // 强引用，造成环形引用
        
        std::cout << "引用计数 - node1: " << node1.use_count() 
                  << ", node2: " << node2.use_count() 
                  << ", node3: " << node3.use_count() << std::endl;
        
        node1->showConnections();
        node2->showConnections();
        node3->showConnections();
        
        std::cout << "即将离开作用域（可能发生内存泄漏）..." << std::endl;
    }
    
    std::cout << "\n--- 正确的版本（使用weak_ptr作为反向引用） ---" << std::endl;
    {
        auto node1 = std::make_shared<ListNode>(10);
        auto node2 = std::make_shared<ListNode>(20);
        auto node3 = std::make_shared<ListNode>(30);
        
        // 建立正确的引用关系：前向使用shared_ptr，反向使用weak_ptr
        node1->setNext(node2);
        node2->setPrevWeak(node1);  // 弱引用，不造成环形引用
        node2->setNext(node3);
        node3->setPrevWeak(node2);  // 弱引用，不造成环形引用
        
        std::cout << "引用计数 - node1: " << node1.use_count() 
                  << ", node2: " << node2.use_count() 
                  << ", node3: " << node3.use_count() << std::endl;
        
        node1->showConnections();
        node2->showConnections();
        node3->showConnections();
        
        std::cout << "即将离开作用域（应该正确释放）..." << std::endl;
    }
    
    std::cout << "双向链表演示完成" << std::endl;
}

// 观察者模式中的环形引用问题
class Subject;
class SafeSubject;

class Observer {
public:
    Observer(const std::string& name) : name_(name) {
        std::cout << "Observer [" << name_ << "] 构造函数调用" << std::endl;
    }
    
    ~Observer() {
        std::cout << "Observer [" << name_ << "] 析构函数调用" << std::endl;
    }
    
    void setSubject(std::shared_ptr<Subject> subject) {
        subject_ = subject; // 使用weak_ptr避免环形引用
    }
    
    void setSafeSubject(std::shared_ptr<SafeSubject> subject) {
        safe_subject_ = subject; // 使用weak_ptr避免环形引用
    }
    
    void notify(const std::string& message) {
        std::cout << "Observer [" << name_ << "] 收到通知: " << message << std::endl;
        
        // 检查subject是否还存在
        if (auto s = subject_.lock()) {
            std::cout << "  (subject仍然存在)" << std::endl;
        } else if (auto s = safe_subject_.lock()) {
            std::cout << "  (safe_subject仍然存在)" << std::endl;
        } else {
            std::cout << "  (subject已不存在)" << std::endl;
        }
    }
    
    const std::string& getName() const { return name_; }

private:
    std::string name_;
    std::weak_ptr<Subject> subject_;
    std::weak_ptr<SafeSubject> safe_subject_;
};

class Subject {
public:
    Subject(const std::string& name) : name_(name) {
        std::cout << "Subject [" << name_ << "] 构造函数调用" << std::endl;
    }
    
    ~Subject() {
        std::cout << "Subject [" << name_ << "] 析构函数调用" << std::endl;
    }
    
    void addObserver(std::shared_ptr<Observer> observer) {
        observers_.push_back(observer);
        observer->setSubject(shared_from_this());
        std::cout << "Subject [" << name_ << "] 添加观察者: " << observer->getName() << std::endl;
    }
    
    void notifyAll(const std::string& message) {
        std::cout << "Subject [" << name_ << "] 通知所有观察者: " << message << std::endl;
        for (auto& observer : observers_) {
            if (observer) {
                observer->notify(message);
            }
        }
    }

private:
    std::string name_;
    std::vector<std::shared_ptr<Observer>> observers_;
};

// 需要继承enable_shared_from_this以使用shared_from_this()
class SafeSubject : public std::enable_shared_from_this<SafeSubject> {
public:
    SafeSubject(const std::string& name) : name_(name) {
        std::cout << "SafeSubject [" << name_ << "] 构造函数调用" << std::endl;
    }
    
    ~SafeSubject() {
        std::cout << "SafeSubject [" << name_ << "] 析构函数调用" << std::endl;
    }
    
    void addObserver(std::shared_ptr<Observer> observer) {
        observers_.push_back(observer);
        observer->setSafeSubject(shared_from_this());
        std::cout << "SafeSubject [" << name_ << "] 添加观察者: " << observer->getName() << std::endl;
    }
    
    void notifyAll(const std::string& message) {
        std::cout << "SafeSubject [" << name_ << "] 通知所有观察者: " << message << std::endl;
        for (auto& observer : observers_) {
            if (observer) {
                observer->notify(message);
            }
        }
    }

private:
    std::string name_;
    std::vector<std::shared_ptr<Observer>> observers_;
};

void demonstrateObserverPattern() {
    std::cout << "\n=== 观察者模式中的环形引用处理 ===" << std::endl;
    
    {
        auto subject = std::make_shared<SafeSubject>("新闻发布者");
        auto observer1 = std::make_shared<Observer>("订阅者1");
        auto observer2 = std::make_shared<Observer>("订阅者2");
        
        subject->addObserver(observer1);
        subject->addObserver(observer2);
        
        subject->notifyAll("重要新闻更新");
        
        std::cout << "即将离开作用域..." << std::endl;
    }
    
    std::cout << "观察者模式演示完成" << std::endl;
}

int main() {
    std::cout << "C++ 智能指针实验 - 环形引用和weak_ptr解决方案" << std::endl;
    std::cout << "================================================" << std::endl;
    
    demonstrateCircularReferenceProblem();
    demonstrateWeakPtrSolution();
    demonstrateLinkedListCircularReference();
    demonstrateObserverPattern();
    
    std::cout << "\n程序结束" << std::endl;
    return 0;
}
