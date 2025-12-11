/*
 * 套接字通信示例程序
 * 演示 Unix Domain Socket 和 Internet Socket 的使用
 * 编译: gcc -o socket_demo 5_socket_demo.c -lpthread
 * 运行: ./socket_demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <netdb.h>

// 颜色定义
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define RESET   "\033[0m"

// 常量定义
#define UNIX_SOCKET_PATH "/tmp/demo_socket"
#define INET_PORT 8888
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 5

// 全局变量
static int server_running = 1;

void print_header(const char *title) {
    printf("\n" BLUE "=== %s ===" RESET "\n", title);
}

void print_success(const char *msg) {
    printf(GREEN "[成功] %s" RESET "\n", msg);
}

void print_error(const char *msg) {
    printf(RED "[错误] %s" RESET "\n", msg);
}

void print_info(const char *msg) {
    printf(YELLOW "[信息] %s" RESET "\n", msg);
}

void print_warning(const char *msg) {
    printf(MAGENTA "[警告] %s" RESET "\n", msg);
}

// 信号处理函数
void signal_handler(int sig) {
    printf("\n收到信号 %d，正在停止服务器...\n", sig);
    server_running = 0;
}

/*
 * Unix Domain Socket 演示 - 服务器端
 */
void unix_socket_server() {
    int server_fd, client_fd;
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    
    print_info("Unix Socket 服务器启动...");
    
    // 删除可能存在的旧套接字文件
    unlink(UNIX_SOCKET_PATH);
    
    // 创建套接字
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        print_error("创建 Unix Socket 失败");
        perror("socket");
        return;
    }
    
    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, UNIX_SOCKET_PATH, sizeof(server_addr.sun_path) - 1);
    
    // 绑定套接字
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        print_error("绑定 Unix Socket 失败");
        perror("bind");
        close(server_fd);
        return;
    }
    
    // 监听连接
    if (listen(server_fd, MAX_CLIENTS) == -1) {
        print_error("监听 Unix Socket 失败");
        perror("listen");
        close(server_fd);
        unlink(UNIX_SOCKET_PATH);
        return;
    }
    
    printf(GREEN "Unix Socket 服务器监听: %s" RESET "\n", UNIX_SOCKET_PATH);
    
    // 接受客户端连接
    client_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd == -1) {
        print_error("接受客户端连接失败");
        perror("accept");
    } else {
        print_success("客户端连接成功");
        
        // 接收数据
        while ((bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
            buffer[bytes_received] = '\0';
            printf(CYAN "服务器收到: %s" RESET "\n", buffer);
            
            // 发送回复
            char reply[BUFFER_SIZE];
            int reply_len = snprintf(reply, sizeof(reply), "Echo: %s", buffer);
            if (reply_len >= (int)sizeof(reply)) {
                reply[sizeof(reply)-1] = '\0'; // 确保字符串终止
            }
            send(client_fd, reply, strlen(reply), 0);
            
            if (strcmp(buffer, "quit") == 0) {
                break;
            }
        }
        
        close(client_fd);
        print_info("客户端连接关闭");
    }
    
    close(server_fd);
    unlink(UNIX_SOCKET_PATH);
    print_success("Unix Socket 服务器关闭");
}

/*
 * Unix Domain Socket 演示 - 客户端
 */
void unix_socket_client() {
    int client_fd;
    struct sockaddr_un server_addr;
    char reply[BUFFER_SIZE];
    ssize_t bytes_sent, bytes_received;
    
    print_info("Unix Socket 客户端启动...");
    
    // 等待服务器启动
    sleep(1);
    
    // 创建套接字
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        print_error("创建客户端套接字失败");
        perror("socket");
        return;
    }
    
    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, UNIX_SOCKET_PATH, sizeof(server_addr.sun_path) - 1);
    
    // 连接服务器
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        print_error("连接服务器失败");
        perror("connect");
        close(client_fd);
        return;
    }
    
    print_success("连接到 Unix Socket 服务器");
    
    // 发送测试消息
    const char* messages[] = {
        "Hello Unix Socket!",
        "This is message 2",
        "Testing communication",
        "quit"
    };
    
    for (int i = 0; i < 4; i++) {
        // 发送消息
        bytes_sent = send(client_fd, messages[i], strlen(messages[i]), 0);
        if (bytes_sent == -1) {
            print_error("发送消息失败");
            break;
        }
        
        printf(GREEN "客户端发送: %s" RESET "\n", messages[i]);
        
        // 接收回复
        bytes_received = recv(client_fd, reply, BUFFER_SIZE - 1, 0);
        if (bytes_received > 0) {
            reply[bytes_received] = '\0';
            printf(CYAN "客户端收到: %s" RESET "\n", reply);
        }
        
        sleep(1);
    }
    
    close(client_fd);
    print_success("Unix Socket 客户端关闭");
}

/*
 * Internet Socket 演示 - 服务器端
 */
void inet_socket_server() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];
    ssize_t bytes_received;
    int opt = 1;
    
    print_info("Internet Socket 服务器启动...");
    
    // 创建套接字
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        print_error("创建 Internet Socket 失败");
        perror("socket");
        return;
    }
    
    // 设置套接字选项，允许地址重用
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        print_warning("设置套接字选项失败");
        perror("setsockopt");
    }
    
    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // 监听所有接口
    server_addr.sin_port = htons(INET_PORT);
    
    // 绑定套接字
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        print_error("绑定 Internet Socket 失败");
        perror("bind");
        close(server_fd);
        return;
    }
    
    // 监听连接
    if (listen(server_fd, MAX_CLIENTS) == -1) {
        print_error("监听 Internet Socket 失败");
        perror("listen");
        close(server_fd);
        return;
    }
    
    printf(GREEN "Internet Socket 服务器监听端口: %d" RESET "\n", INET_PORT);
    
    // 接受客户端连接
    client_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd == -1) {
        print_error("接受客户端连接失败");
        perror("accept");
    } else {
        // 获取客户端IP地址
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf(GREEN "客户端连接成功: %s:%d" RESET "\n", client_ip, ntohs(client_addr.sin_port));
        
        // 接收数据
        while ((bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
            buffer[bytes_received] = '\0';
            printf(CYAN "服务器收到来自 %s: %s" RESET "\n", client_ip, buffer);
            
            // 发送回复
            char reply[BUFFER_SIZE];
            int reply_len = snprintf(reply, sizeof(reply), "Server Echo: %s", buffer);
            if (reply_len >= (int)sizeof(reply)) {
                reply[sizeof(reply)-1] = '\0'; // 确保字符串终止
            }
            send(client_fd, reply, strlen(reply), 0);
            
            if (strcmp(buffer, "quit") == 0) {
                break;
            }
        }
        
        close(client_fd);
        printf(GREEN "客户端 %s 断开连接" RESET "\n", client_ip);
    }
    
    close(server_fd);
    print_success("Internet Socket 服务器关闭");
}

/*
 * Internet Socket 演示 - 客户端
 */
void inet_socket_client() {
    int client_fd;
    struct sockaddr_in server_addr;
    char reply[BUFFER_SIZE];
    ssize_t bytes_sent, bytes_received;
    
    print_info("Internet Socket 客户端启动...");
    
    // 等待服务器启动
    sleep(1);
    
    // 创建套接字
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        print_error("创建客户端套接字失败");
        perror("socket");
        return;
    }
    
    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(INET_PORT);
    
    // 将字符串IP地址转换为网络地址
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        print_error("无效的服务器地址");
        close(client_fd);
        return;
    }
    
    // 连接服务器
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        print_error("连接服务器失败");
        perror("connect");
        close(client_fd);
        return;
    }
    
    print_success("连接到 Internet Socket 服务器");
    
    // 发送测试消息
    const char* messages[] = {
        "Hello Internet Socket!",
        "TCP communication test",
        "Message from client",
        "quit"
    };
    
    for (int i = 0; i < 4; i++) {
        // 发送消息
        bytes_sent = send(client_fd, messages[i], strlen(messages[i]), 0);
        if (bytes_sent == -1) {
            print_error("发送消息失败");
            break;
        }
        
        printf(GREEN "客户端发送: %s" RESET "\n", messages[i]);
        
        // 接收回复
        bytes_received = recv(client_fd, reply, BUFFER_SIZE - 1, 0);
        if (bytes_received > 0) {
            reply[bytes_received] = '\0';
            printf(CYAN "客户端收到: %s" RESET "\n", reply);
        }
        
        sleep(1);
    }
    
    close(client_fd);
    print_success("Internet Socket 客户端关闭");
}

/*
 * UDP Socket 演示
 */
void demo_udp_socket() {
    print_header("UDP Socket 演示");
    
    pid_t pid = fork();
    
    if (pid == -1) {
        print_error("创建子进程失败");
        return;
    }
    
    if (pid == 0) {
        // 子进程 - UDP 服务器
        int server_fd;
        struct sockaddr_in server_addr, client_addr;
        socklen_t client_len;
        char buffer[BUFFER_SIZE];
        ssize_t bytes_received;
        
        print_info("UDP 服务器启动...");
        
        // 创建UDP套接字
        server_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (server_fd == -1) {
            print_error("创建UDP套接字失败");
            exit(1);
        }
        
        // 设置服务器地址
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(INET_PORT + 1);
        
        // 绑定套接字
        if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
            print_error("绑定UDP套接字失败");
            close(server_fd);
            exit(1);
        }
        
        printf(GREEN "UDP 服务器监听端口: %d" RESET "\n", INET_PORT + 1);
        
        // 接收数据
        for (int i = 0; i < 3; i++) {
            client_len = sizeof(client_addr);
            bytes_received = recvfrom(server_fd, buffer, BUFFER_SIZE - 1, 0,
                                    (struct sockaddr*)&client_addr, &client_len);
            
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
                
                printf(CYAN "UDP服务器收到来自 %s:%d: %s" RESET "\n", 
                       client_ip, ntohs(client_addr.sin_port), buffer);
                
                // 发送回复
                char reply[BUFFER_SIZE];
                int reply_len = snprintf(reply, sizeof(reply), "UDP Echo: %s", buffer);
                if (reply_len >= (int)sizeof(reply)) {
                    reply[sizeof(reply)-1] = '\0'; // 确保字符串终止
                }
                sendto(server_fd, reply, strlen(reply), 0,
                       (struct sockaddr*)&client_addr, client_len);
            }
        }
        
        close(server_fd);
        print_success("UDP 服务器关闭");
        exit(0);
        
    } else {
        // 父进程 - UDP 客户端
        int client_fd;
        struct sockaddr_in server_addr;
        char buffer[BUFFER_SIZE];
        char reply[BUFFER_SIZE];
        socklen_t server_len;
        ssize_t bytes_sent, bytes_received;
        
        sleep(1);  // 等待服务器启动
        print_info("UDP 客户端启动...");
        
        // 创建UDP套接字
        client_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (client_fd == -1) {
            print_error("创建UDP客户端套接字失败");
            kill(pid, SIGTERM);
            return;
        }
        
        // 设置服务器地址
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(INET_PORT + 1);
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
        
        // 发送测试消息
        const char* messages[] = {
            "UDP Message 1",
            "UDP Message 2", 
            "UDP Message 3"
        };
        
        for (int i = 0; i < 3; i++) {
            // 发送消息
            server_len = sizeof(server_addr);
            bytes_sent = sendto(client_fd, messages[i], strlen(messages[i]), 0,
                              (struct sockaddr*)&server_addr, server_len);
            
            if (bytes_sent == -1) {
                print_error("发送UDP消息失败");
                break;
            }
            
            printf(GREEN "UDP客户端发送: %s" RESET "\n", messages[i]);
            
            // 接收回复
            bytes_received = recvfrom(client_fd, reply, BUFFER_SIZE - 1, 0,
                                    (struct sockaddr*)&server_addr, &server_len);
            
            if (bytes_received > 0) {
                reply[bytes_received] = '\0';
                printf(CYAN "UDP客户端收到: %s" RESET "\n", reply);
            }
            
            sleep(1);
        }
        
        close(client_fd);
        print_success("UDP 客户端关闭");
        
        // 等待子进程结束
        int status;
        wait(&status);
    }
}

/*
 * 演示 Unix Domain Socket
 */
void demo_unix_domain_socket() {
    print_header("Unix Domain Socket 演示");
    
    pid_t pid = fork();
    
    if (pid == -1) {
        print_error("创建子进程失败");
        return;
    }
    
    if (pid == 0) {
        // 子进程 - 服务器
        unix_socket_server();
        exit(0);
    } else {
        // 父进程 - 客户端
        unix_socket_client();
        
        // 等待子进程结束
        int status;
        wait(&status);
        print_success("Unix Domain Socket 演示完成");
    }
}

/*
 * 演示 Internet Socket
 */
void demo_internet_socket() {
    print_header("Internet Socket 演示");
    
    pid_t pid = fork();
    
    if (pid == -1) {
        print_error("创建子进程失败");
        return;
    }
    
    if (pid == 0) {
        // 子进程 - 服务器
        inet_socket_server();
        exit(0);
    } else {
        // 父进程 - 客户端
        inet_socket_client();
        
        // 等待子进程结束
        int status;
        wait(&status);
        print_success("Internet Socket 演示完成");
    }
}

/*
 * 显示套接字特性和类型
 */
void show_socket_characteristics() {
    print_header("套接字特性和类型");
    
    printf("📋 " YELLOW "套接字类型:" RESET "\n");
    printf("  • Unix Domain Socket (AF_UNIX)\n");
    printf("    - 用于同一主机上的进程通信\n");
    printf("    - 使用文件系统路径作为地址\n");
    printf("    - 性能高，无网络开销\n");
    printf("  • Internet Socket (AF_INET)\n");
    printf("    - 用于网络通信\n");
    printf("    - 支持TCP和UDP协议\n");
    printf("    - 可跨主机通信\n");
    
    printf("\n🔧 " YELLOW "传输协议:" RESET "\n");
    printf("  • TCP (SOCK_STREAM)\n");
    printf("    - 可靠的、面向连接的\n");
    printf("    - 保证数据顺序和完整性\n");
    printf("    - 适合需要可靠传输的应用\n");
    printf("  • UDP (SOCK_DGRAM)\n");
    printf("    - 不可靠的、无连接的\n");
    printf("    - 速度快，开销小\n");
    printf("    - 适合实时性要求高的应用\n");
    
    printf("\n✨ " YELLOW "套接字特性:" RESET "\n");
    printf("  • 双向通信\n");
    printf("  • 支持多种地址族\n");
    printf("  • 可以跨网络通信\n");
    printf("  • 支持异步和非阻塞操作\n");
    printf("  • 可以传输任意数据\n");
    
    printf("\n🆚 " YELLOW "与其他IPC比较:" RESET "\n");
    printf("  • vs 管道: 更灵活，支持网络通信\n");
    printf("  • vs 消息队列: 更通用，跨平台\n");
    printf("  • vs 共享内存: 有数据拷贝开销，但更安全\n");
    printf("  • vs 信号量: 主要用于数据传输而非同步\n");
    
    printf("\n🌐 " YELLOW "应用场景:" RESET "\n");
    printf("  • 客户端-服务器架构\n");
    printf("  • 分布式系统通信\n");
    printf("  • 微服务间通信\n");
    printf("  • 实时数据传输\n");
    printf("  • 跨语言进程通信\n");
}

int main() {
    printf(BLUE "🚀 Linux 进程间通信 - 套接字演示程序\n" RESET);
    printf("本程序演示 Unix Domain Socket 和 Internet Socket 的使用\n");
    
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, SIG_IGN);  // 忽略 SIGPIPE 信号
    
    // 演示各种套接字通信
    demo_unix_domain_socket();
    
    printf("\n按 Enter 键继续...");
    getchar();
    
    demo_internet_socket();
    
    printf("\n按 Enter 键继续...");
    getchar();
    
    demo_udp_socket();
    
    printf("\n按 Enter 键继续...");
    getchar();
    
    show_socket_characteristics();
    
    printf("\n" GREEN "🎉 套接字演示完成！" RESET "\n");
    printf("💡 " YELLOW "学习要点:" RESET "\n");
    printf("  1. 套接字是最通用的IPC机制\n");
    printf("  2. 支持本地和网络通信\n");
    printf("  3. TCP提供可靠传输，UDP提供高效传输\n");
    printf("  4. 适合构建分布式系统\n");
    
    return 0;
}
