#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <fstream>

// 示例类：文件资源
class FileResource {
public:
    FileResource(const std::string& filename) : filename_(filename) {
        std::cout << "FileResource [" << filename_ << "] 构造函数调用" << std::endl;
        // 模拟文件打开
        file_handle_ = fopen(filename_.c_str(), "w");
        if (file_handle_) {
            std::cout << "文件 [" << filename_ << "] 成功打开" << std::endl;
        }
    }
    
    ~FileResource() {
        std::cout << "FileResource [" << filename_ << "] 析构函数调用" << std::endl;
        if (file_handle_) {
            fclose(file_handle_);
            std::cout << "文件 [" << filename_ << "] 已关闭" << std::endl;
        }
    }
    
    void write(const std::string& data) {
        if (file_handle_) {
            fprintf(file_handle_, "%s\n", data.c_str());
            std::cout << "向文件 [" << filename_ << "] 写入: " << data << std::endl;
        }
    }
    
    const std::string& getFilename() const { return filename_; }
    
    // 禁止拷贝构造和拷贝赋值
    FileResource(const FileResource&) = delete;
    FileResource& operator=(const FileResource&) = delete;

private:
    std::string filename_;
    FILE* file_handle_;
};

// 演示unique_ptr的基本使用
void demonstrateBasicUsage() {
    std::cout << "\n=== unique_ptr 基本使用演示 ===" << std::endl;
    
    // 创建unique_ptr的几种方式
    std::unique_ptr<FileResource> file1 = std::make_unique<FileResource>("test1.txt");
    std::unique_ptr<FileResource> file2(new FileResource("test2.txt"));
    
    // 使用资源
    file1->write("Hello from file1");
    file2->write("Hello from file2");
    
    // 检查是否为空
    if (file1) {
        std::cout << "file1 不为空，文件名: " << file1->getFilename() << std::endl;
    }
    
    // 释放资源
    file1.reset(); // 手动释放
    std::cout << "file1 已手动释放" << std::endl;
    
    // file2会在离开作用域时自动释放
}

// 演示所有权转移（移动语义）
std::unique_ptr<FileResource> createFile(const std::string& filename) {
    std::cout << "在函数中创建文件资源: " << filename << std::endl;
    return std::make_unique<FileResource>(filename);
}

void processFile(std::unique_ptr<FileResource> file) {
    std::cout << "处理文件: " << file->getFilename() << std::endl;
    file->write("数据处理完成");
    // 函数结束时，file会自动释放资源
}

void demonstrateOwnershipTransfer() {
    std::cout << "\n=== 所有权转移演示 ===" << std::endl;
    
    // 1. 从函数返回unique_ptr（移动构造）
    auto file1 = createFile("transfer1.txt");
    std::cout << "从函数获得文件资源: " << file1->getFilename() << std::endl;
    
    // 2. 移动赋值
    std::unique_ptr<FileResource> file2;
    file2 = std::move(file1); // 所有权从file1转移到file2
    
    if (!file1) {
        std::cout << "file1 现在为空（所有权已转移）" << std::endl;
    }
    if (file2) {
        std::cout << "file2 现在拥有资源: " << file2->getFilename() << std::endl;
    }
    
    // 3. 传递给函数（移动语义）
    auto file3 = std::make_unique<FileResource>("transfer3.txt");
    std::cout << "将要传递文件给处理函数: " << file3->getFilename() << std::endl;
    processFile(std::move(file3)); // 所有权转移给函数
    
    if (!file3) {
        std::cout << "file3 现在为空（所有权已转移给函数）" << std::endl;
    }
}

// 演示容器中的unique_ptr
void demonstrateContainerUsage() {
    std::cout << "\n=== 容器中的unique_ptr演示 ===" << std::endl;
    
    std::vector<std::unique_ptr<FileResource>> files;
    
    // 添加元素到容器
    files.push_back(std::make_unique<FileResource>("container1.txt"));
    files.push_back(std::make_unique<FileResource>("container2.txt"));
    files.push_back(std::make_unique<FileResource>("container3.txt"));
    
    std::cout << "容器中有 " << files.size() << " 个文件资源" << std::endl;
    
    // 使用容器中的资源
    for (size_t i = 0; i < files.size(); ++i) {
        files[i]->write("容器中的数据 " + std::to_string(i+1));
    }
    
    // 移除中间元素
    std::cout << "移除第2个文件资源" << std::endl;
    files.erase(files.begin() + 1);
    
    std::cout << "移除后，容器中还有 " << files.size() << " 个文件资源" << std::endl;
    
    // 清空容器
    files.clear();
    std::cout << "容器已清空，所有资源已释放" << std::endl;
}

// 演示自定义删除器
class NetworkConnection {
public:
    NetworkConnection(const std::string& host, int port) 
        : host_(host), port_(port), connected_(true) {
        std::cout << "网络连接建立: " << host_ << ":" << port_ << std::endl;
    }
    
    ~NetworkConnection() {
        std::cout << "NetworkConnection 析构函数调用" << std::endl;
        disconnect();
    }
    
    void disconnect() {
        if (connected_) {
            std::cout << "断开网络连接: " << host_ << ":" << port_ << std::endl;
            connected_ = false;
        }
    }
    
    void send(const std::string& data) {
        if (connected_) {
            std::cout << "发送数据到 " << host_ << ":" << port_ << " -> " << data << std::endl;
        }
    }

private:
    std::string host_;
    int port_;
    bool connected_;
};

void demonstrateCustomDeleter() {
    std::cout << "\n=== 自定义删除器演示 ===" << std::endl;
    
    // 自定义删除器
    auto customDeleter = [](NetworkConnection* conn) {
        std::cout << "自定义删除器被调用" << std::endl;
        conn->disconnect(); // 确保连接被正确关闭
        delete conn;
    };
    
    {
        std::unique_ptr<NetworkConnection, decltype(customDeleter)> 
            connection(new NetworkConnection("192.168.1.100", 8080), customDeleter);
        
        connection->send("Hello Server");
        
        // 离开作用域时，自定义删除器会被调用
    }
    
    std::cout << "连接已通过自定义删除器清理" << std::endl;
}

// 演示release和get方法
void demonstrateReleaseAndGet() {
    std::cout << "\n=== release和get方法演示 ===" << std::endl;
    
    auto file = std::make_unique<FileResource>("release_demo.txt");
    
    // 获取原始指针（但不释放所有权）
    FileResource* rawPtr = file.get();
    std::cout << "通过get()获得原始指针，文件名: " << rawPtr->getFilename() << std::endl;
    
    // 释放所有权
    FileResource* releasedPtr = file.release();
    
    if (!file) {
        std::cout << "unique_ptr现在为空（所有权已释放）" << std::endl;
    }
    
    std::cout << "通过release()获得的指针，文件名: " << releasedPtr->getFilename() << std::endl;
    
    // 注意：现在需要手动删除对象
    delete releasedPtr;
    std::cout << "手动删除了释放的对象" << std::endl;
}

// 工厂函数示例
template<typename T, typename... Args>
std::unique_ptr<T> make_resource(Args&&... args) {
    std::cout << "工厂函数创建资源" << std::endl;
    return std::make_unique<T>(std::forward<Args>(args)...);
}

void demonstrateFactory() {
    std::cout << "\n=== 工厂函数演示 ===" << std::endl;
    
    auto file = make_resource<FileResource>("factory_created.txt");
    file->write("由工厂函数创建");
    
    auto connection = make_resource<NetworkConnection>("localhost", 9090);
    connection->send("Factory created connection");
}

int main() {
    std::cout << "C++ 智能指针实验 - unique_ptr 演示" << std::endl;
    std::cout << "====================================" << std::endl;
    
    demonstrateBasicUsage();
    demonstrateOwnershipTransfer();
    demonstrateContainerUsage();
    demonstrateCustomDeleter();
    demonstrateReleaseAndGet();
    demonstrateFactory();
    
    std::cout << "\n程序结束" << std::endl;
    return 0;
}
