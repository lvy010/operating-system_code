/*
 * 消息队列通信示例程序
 * 演示 System V 消息队列的使用
 * 编译: gcc -o msgqueue_demo 2_msgqueue_demo.c
 * 运行: ./msgqueue_demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

// 颜色定义
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define RESET   "\033[0m"

// 消息结构体
struct message {
    long msg_type;          // 消息类型，必须大于0
    char msg_text[256];     // 消息内容
    int sender_pid;         // 发送者进程ID
    int msg_id;             // 消息序号
};

// 全局变量
static int msgq_id = -1;

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

// 清理函数
void cleanup_msgqueue() {
    if (msgq_id != -1) {
        if (msgctl(msgq_id, IPC_RMID, NULL) == -1) {
            print_error("删除消息队列失败");
            perror("msgctl");
        } else {
            print_info("消息队列已清理");
        }
    }
}

// 信号处理函数
void signal_handler(int sig) {
    printf("\n收到信号 %d，正在清理资源...\n", sig);
    cleanup_msgqueue();
    exit(0);
}

/*
 * 创建消息队列
 */
int create_message_queue() {
    key_t key;
    
    // 生成唯一的键值
    key = ftok("/tmp", 'M');  // 使用文件路径和项目ID生成键值
    if (key == -1) {
        print_error("生成键值失败");
        perror("ftok");
        return -1;
    }
    
    printf("生成的键值: 0x%x\n", key);
    
    // 创建消息队列
    msgq_id = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
    if (msgq_id == -1) {
        if (errno == EEXIST) {
            print_warning("消息队列已存在，尝试获取现有队列");
            msgq_id = msgget(key, 0666);
            if (msgq_id == -1) {
                print_error("获取现有消息队列失败");
                perror("msgget");
                return -1;
            }
        } else {
            print_error("创建消息队列失败");
            perror("msgget");
            return -1;
        }
    }
    
    printf("消息队列ID: %d\n", msgq_id);
    print_success("消息队列创建/获取成功");
    
    return msgq_id;
}

/*
 * 发送消息
 */
int send_message(long msg_type, const char *text, int sender_pid, int msg_id) {
    struct message msg;
    
    msg.msg_type = msg_type;
    strncpy(msg.msg_text, text, sizeof(msg.msg_text) - 1);
    msg.msg_text[sizeof(msg.msg_text) - 1] = '\0';
    msg.sender_pid = sender_pid;
    msg.msg_id = msg_id;
    
    if (msgsnd(msgq_id, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        print_error("发送消息失败");
        perror("msgsnd");
        return -1;
    }
    
    printf(GREEN "发送消息 [类型:%ld, ID:%d]: %s" RESET "\n", 
           msg_type, msg_id, text);
    return 0;
}

/*
 * 接收消息
 */
int receive_message(long msg_type, int flags) {
    struct message msg;
    ssize_t msg_size;
    
    msg_size = msgrcv(msgq_id, &msg, sizeof(msg) - sizeof(long), msg_type, flags);
    if (msg_size == -1) {
        if (errno == ENOMSG && (flags & IPC_NOWAIT)) {
            print_info("没有可用消息 (非阻塞模式)");
            return 0;
        } else {
            print_error("接收消息失败");
            perror("msgrcv");
            return -1;
        }
    }
    
    printf(CYAN "收到消息 [类型:%ld, 发送者:%d, ID:%d]: %s" RESET "\n",
           msg.msg_type, msg.sender_pid, msg.msg_id, msg.msg_text);
    return 1;
}

/*
 * 显示消息队列状态
 */
void show_msgqueue_status() {
    struct msqid_ds buf;
    
    if (msgctl(msgq_id, IPC_STAT, &buf) == -1) {
        print_error("获取消息队列状态失败");
        perror("msgctl");
        return;
    }
    
    print_header("消息队列状态信息");
    printf("队列ID: %d\n", msgq_id);
    printf("当前消息数: %lu\n", buf.msg_qnum);
    printf("队列中的字节数: %lu\n", (unsigned long)buf.msg_qbytes);
    printf("最大字节数: %lu\n", buf.msg_qbytes);
    printf("最后发送消息的进程ID: %d\n", buf.msg_lspid);
    printf("最后接收消息的进程ID: %d\n", buf.msg_lrpid);
    printf("最后发送时间: %s", ctime(&buf.msg_stime));
    printf("最后接收时间: %s", ctime(&buf.msg_rtime));
    printf("最后修改时间: %s", ctime(&buf.msg_ctime));
}

/*
 * 演示基本的消息发送和接收
 */
void demo_basic_messaging() {
    print_header("基本消息队列通信演示");
    
    pid_t pid = fork();
    
    if (pid == -1) {
        print_error("创建子进程失败");
        perror("fork");
        return;
    }
    
    if (pid == 0) {
        // 子进程 - 接收消息
        print_info("子进程: 等待接收消息...");
        sleep(1);  // 稍微延迟
        
        // 接收类型为1的消息
        if (receive_message(1, 0) > 0) {
            print_success("子进程成功接收消息");
        }
        
        // 发送回复消息
        send_message(2, "Reply from child process", getpid(), 1001);
        
        exit(0);
        
    } else {
        // 父进程 - 发送消息
        print_info("父进程: 发送消息...");
        
        // 发送类型为1的消息
        send_message(1, "Hello from parent process", getpid(), 1000);
        
        // 接收子进程的回复
        print_info("父进程: 等待子进程回复...");
        if (receive_message(2, 0) > 0) {
            print_success("父进程收到回复");
        }
        
        // 等待子进程结束
        int status;
        wait(&status);
        print_success("基本消息通信完成");
    }
}

/*
 * 演示优先级消息处理
 */
void demo_priority_messaging() {
    print_header("优先级消息处理演示");
    
    pid_t pid = fork();
    
    if (pid == -1) {
        print_error("创建子进程失败");
        perror("fork");
        return;
    }
    
    if (pid == 0) {
        // 子进程 - 按优先级接收消息
        print_info("子进程: 等待接收消息...");
        sleep(2);  // 让父进程先发送所有消息
        
        print_info("按优先级接收消息 (高优先级先接收):");
        
        // 接收高优先级消息 (类型3)
        receive_message(3, IPC_NOWAIT);
        
        // 接收中等优先级消息 (类型2)
        receive_message(2, IPC_NOWAIT);
        
        // 接收低优先级消息 (类型1)
        receive_message(1, IPC_NOWAIT);
        
        exit(0);
        
    } else {
        // 父进程 - 发送不同优先级的消息
        print_info("父进程: 发送不同优先级的消息...");
        
        // 发送低优先级消息 (类型1)
        send_message(1, "低优先级消息", getpid(), 2001);
        
        // 发送高优先级消息 (类型3)
        send_message(3, "高优先级消息", getpid(), 2003);
        
        // 发送中等优先级消息 (类型2)
        send_message(2, "中等优先级消息", getpid(), 2002);
        
        show_msgqueue_status();
        
        // 等待子进程结束
        int status;
        wait(&status);
        print_success("优先级消息处理完成");
    }
}

/*
 * 演示多生产者多消费者模式
 */
void demo_multiple_producers_consumers() {
    print_header("多生产者多消费者模式演示");
    
    const int num_producers = 2;
    const int num_consumers = 2;
    pid_t pids[num_producers + num_consumers];
    int i;
    
    // 创建生产者进程
    for (i = 0; i < num_producers; i++) {
        pids[i] = fork();
        
        if (pids[i] == -1) {
            print_error("创建生产者进程失败");
            continue;
        }
        
        if (pids[i] == 0) {
            // 生产者进程
            printf(GREEN "生产者 %d (PID:%d) 开始工作" RESET "\n", i+1, getpid());
            
            for (int j = 0; j < 3; j++) {
                char msg[256];
                snprintf(msg, sizeof(msg), "来自生产者%d的消息%d", i+1, j+1);
                send_message(10, msg, getpid(), 3000 + i*10 + j);
                sleep(1);
            }
            
            printf(GREEN "生产者 %d 完成工作" RESET "\n", i+1);
            exit(0);
        }
    }
    
    // 创建消费者进程
    for (i = 0; i < num_consumers; i++) {
        pids[num_producers + i] = fork();
        
        if (pids[num_producers + i] == -1) {
            print_error("创建消费者进程失败");
            continue;
        }
        
        if (pids[num_producers + i] == 0) {
            // 消费者进程
            printf(CYAN "消费者 %d (PID:%d) 开始工作" RESET "\n", i+1, getpid());
            
            for (int j = 0; j < 3; j++) {
                if (receive_message(10, 0) > 0) {
                    printf(CYAN "消费者 %d 处理了一条消息" RESET "\n", i+1);
                }
                sleep(1);
            }
            
            printf(CYAN "消费者 %d 完成工作" RESET "\n", i+1);
            exit(0);
        }
    }
    
    // 等待所有子进程结束
    for (i = 0; i < num_producers + num_consumers; i++) {
        if (pids[i] > 0) {
            int status;
            waitpid(pids[i], &status, 0);
        }
    }
    
    show_msgqueue_status();
    print_success("多生产者多消费者演示完成");
}

/*
 * 演示非阻塞消息操作
 */
void demo_nonblocking_operations() {
    print_header("非阻塞消息操作演示");
    
    print_info("尝试非阻塞接收 (队列为空时):");
    receive_message(999, IPC_NOWAIT);  // 尝试接收不存在的消息类型
    
    print_info("发送测试消息:");
    send_message(100, "测试非阻塞接收", getpid(), 4000);
    
    print_info("非阻塞接收刚发送的消息:");
    receive_message(100, IPC_NOWAIT);
    
    print_success("非阻塞操作演示完成");
}

/*
 * 显示消息队列特性和限制
 */
void show_msgqueue_characteristics() {
    print_header("消息队列特性和限制");
    
    printf("📋 " YELLOW "消息队列特性:" RESET "\n");
    printf("  • 消息具有类型，可以有选择地接收\n");
    printf("  • 消息在内核中缓存，发送方不必等待\n");
    printf("  • 支持优先级处理 (按消息类型)\n");
    printf("  • 可以实现多对多通信\n");
    printf("  • 消息边界得到保持\n");
    
    printf("\n📏 " YELLOW "消息队列限制:" RESET "\n");
    printf("  • 消息大小有限制 (通常几KB到几MB)\n");
    printf("  • 队列中消息数量有限制\n");
    printf("  • 需要显式删除，否则会持续存在\n");
    printf("  • 不适合大量数据传输\n");
    
    // 显示系统限制
    printf("\n💾 " YELLOW "系统限制:" RESET "\n");
    printf("  • 查看系统限制: cat /proc/sys/kernel/msgmax\n");
    printf("  • 查看队列限制: cat /proc/sys/kernel/msgmnb\n");
    printf("  • 查看队列数量限制: cat /proc/sys/kernel/msgmni\n");
}

int main() {
    printf(BLUE "🚀 Linux 进程间通信 - 消息队列演示程序\n" RESET);
    printf("本程序演示 System V 消息队列的各种使用方法\n");
    
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // 创建消息队列
    if (create_message_queue() == -1) {
        return 1;
    }
    
    // 演示各种消息队列操作
    demo_basic_messaging();
    
    printf("\n按 Enter 键继续...");
    getchar();
    
    demo_priority_messaging();
    
    printf("\n按 Enter 键继续...");
    getchar();
    
    demo_multiple_producers_consumers();
    
    printf("\n按 Enter 键继续...");
    getchar();
    
    demo_nonblocking_operations();
    
    printf("\n按 Enter 键继续...");
    getchar();
    
    show_msgqueue_characteristics();
    
    // 清理资源
    cleanup_msgqueue();
    
    printf("\n" GREEN "🎉 消息队列演示完成！" RESET "\n");
    printf("💡 " YELLOW "学习要点:" RESET "\n");
    printf("  1. 消息队列支持消息类型和优先级\n");
    printf("  2. 可以实现多对多通信模式\n");
    printf("  3. 消息在内核中缓存，异步通信\n");
    printf("  4. 需要注意资源清理，避免系统资源泄漏\n");
    
    return 0;
}
