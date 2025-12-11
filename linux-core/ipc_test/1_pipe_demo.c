/*
 * 管道通信示例程序
 * 演示匿名管道和命名管道(FIFO)的使用
 * 编译: gcc -o pipe_demo 1_pipe_demo.c
 * 运行: ./pipe_demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#define BUFFER_SIZE 1024
#define FIFO_NAME "/tmp/demo_fifo"

// 颜色定义
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define RESET   "\033[0m"

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

/*
 * 演示匿名管道 (pipe)
 * 用于父子进程间通信
 */
void demo_anonymous_pipe() {
    print_header("匿名管道 (Anonymous Pipe) 演示");
    
    int pipefd[2];  // 管道文件描述符数组
    pid_t pid;
    char write_msg[] = "Hello from parent process!";
    char read_msg[BUFFER_SIZE];
    
    // 创建管道
    if (pipe(pipefd) == -1) {
        print_error("创建管道失败");
        perror("pipe");
        return;
    }
    
    print_info("管道创建成功");
    printf("管道读端文件描述符: %d\n", pipefd[0]);
    printf("管道写端文件描述符: %d\n", pipefd[1]);
    
    // 创建子进程
    pid = fork();
    
    if (pid == -1) {
        print_error("创建子进程失败");
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }
    
    if (pid == 0) {
        // 子进程 - 读取数据
        print_info("子进程: 准备从管道读取数据");
        
        close(pipefd[1]);  // 关闭写端
        
        ssize_t bytes_read = read(pipefd[0], read_msg, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            read_msg[bytes_read] = '\0';
            printf(GREEN "子进程收到消息: %s" RESET "\n", read_msg);
        } else {
            print_error("子进程读取失败");
        }
        
        close(pipefd[0]);
        exit(0);
        
    } else {
        // 父进程 - 发送数据
        print_info("父进程: 准备向管道写入数据");
        
        close(pipefd[0]);  // 关闭读端
        
        sleep(1);  // 稍微延迟，让子进程准备好
        
        ssize_t bytes_written = write(pipefd[1], write_msg, strlen(write_msg));
        if (bytes_written > 0) {
            printf(GREEN "父进程发送消息: %s" RESET "\n", write_msg);
        } else {
            print_error("父进程写入失败");
        }
        
        close(pipefd[1]);
        
        // 等待子进程结束
        int status;
        wait(&status);
        print_success("匿名管道通信完成");
    }
}

/*
 * 演示命名管道 (FIFO)
 * 可用于无亲缘关系的进程间通信
 */
void demo_named_pipe() {
    print_header("命名管道 (Named Pipe/FIFO) 演示");
    
    pid_t pid;
    char write_msg[] = "Hello from named pipe!";
    char read_msg[BUFFER_SIZE];
    
    // 删除可能存在的旧 FIFO
    unlink(FIFO_NAME);
    
    // 创建命名管道
    if (mkfifo(FIFO_NAME, 0666) == -1) {
        print_error("创建命名管道失败");
        perror("mkfifo");
        return;
    }
    
    print_success("命名管道创建成功");
    printf("FIFO 路径: %s\n", FIFO_NAME);
    
    pid = fork();
    
    if (pid == -1) {
        print_error("创建子进程失败");
        perror("fork");
        unlink(FIFO_NAME);
        return;
    }
    
    if (pid == 0) {
        // 子进程 - 读取数据
        print_info("子进程: 打开 FIFO 进行读取");
        
        int fd = open(FIFO_NAME, O_RDONLY);
        if (fd == -1) {
            print_error("子进程打开 FIFO 失败");
            perror("open");
            exit(1);
        }
        
        ssize_t bytes_read = read(fd, read_msg, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            read_msg[bytes_read] = '\0';
            printf(GREEN "子进程从 FIFO 收到: %s" RESET "\n", read_msg);
        } else {
            print_error("子进程从 FIFO 读取失败");
        }
        
        close(fd);
        exit(0);
        
    } else {
        // 父进程 - 写入数据
        print_info("父进程: 打开 FIFO 进行写入");
        
        sleep(1);  // 确保子进程先打开读端
        
        int fd = open(FIFO_NAME, O_WRONLY);
        if (fd == -1) {
            print_error("父进程打开 FIFO 失败");
            perror("open");
            kill(pid, SIGTERM);
            unlink(FIFO_NAME);
            return;
        }
        
        ssize_t bytes_written = write(fd, write_msg, strlen(write_msg));
        if (bytes_written > 0) {
            printf(GREEN "父进程向 FIFO 发送: %s" RESET "\n", write_msg);
        } else {
            print_error("父进程向 FIFO 写入失败");
        }
        
        close(fd);
        
        // 等待子进程结束
        int status;
        wait(&status);
        
        // 清理 FIFO
        unlink(FIFO_NAME);
        print_success("命名管道通信完成");
    }
}

/*
 * 演示双向管道通信
 */
void demo_bidirectional_pipe() {
    print_header("双向管道通信演示");
    
    int pipe1[2], pipe2[2];  // pipe1: 父->子, pipe2: 子->父
    pid_t pid;
    char parent_msg[] = "Message from parent";
    char child_msg[] = "Reply from child";
    char buffer[BUFFER_SIZE];
    
    // 创建两个管道
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        print_error("创建管道失败");
        perror("pipe");
        return;
    }
    
    print_info("双向管道创建成功");
    
    pid = fork();
    
    if (pid == -1) {
        print_error("创建子进程失败");
        perror("fork");
        return;
    }
    
    if (pid == 0) {
        // 子进程
        close(pipe1[1]);  // 关闭 pipe1 写端
        close(pipe2[0]);  // 关闭 pipe2 读端
        
        // 从父进程读取消息
        ssize_t bytes_read = read(pipe1[0], buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf(GREEN "子进程收到: %s" RESET "\n", buffer);
        }
        
        // 向父进程发送回复
        write(pipe2[1], child_msg, strlen(child_msg));
        printf(GREEN "子进程发送: %s" RESET "\n", child_msg);
        
        close(pipe1[0]);
        close(pipe2[1]);
        exit(0);
        
    } else {
        // 父进程
        close(pipe1[0]);  // 关闭 pipe1 读端
        close(pipe2[1]);  // 关闭 pipe2 写端
        
        // 向子进程发送消息
        write(pipe1[1], parent_msg, strlen(parent_msg));
        printf(GREEN "父进程发送: %s" RESET "\n", parent_msg);
        
        // 从子进程读取回复
        ssize_t bytes_read = read(pipe2[0], buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf(GREEN "父进程收到回复: %s" RESET "\n", buffer);
        }
        
        close(pipe1[1]);
        close(pipe2[0]);
        
        // 等待子进程结束
        int status;
        wait(&status);
        print_success("双向管道通信完成");
    }
}

/*
 * 显示管道的特性和限制
 */
void show_pipe_characteristics() {
    print_header("管道特性和限制");
    
    printf("📋 " YELLOW "管道特性:" RESET "\n");
    printf("  • 半双工通信 (数据只能单向流动)\n");
    printf("  • 只能用于有亲缘关系的进程 (匿名管道)\n");
    printf("  • 命名管道可用于无亲缘关系的进程\n");
    printf("  • 管道是特殊的文件，存在于内存中\n");
    printf("  • 读写操作是原子性的 (小于 PIPE_BUF 字节)\n");
    
    printf("\n📏 " YELLOW "管道限制:" RESET "\n");
    printf("  • 缓冲区大小有限 (通常 64KB)\n");
    printf("  • 写满时写进程阻塞\n");
    printf("  • 读空时读进程阻塞\n");
    printf("  • 写端关闭时读端收到 EOF\n");
    printf("  • 读端关闭时写端收到 SIGPIPE 信号\n");
    
    // 显示系统管道缓冲区大小
    long pipe_buf = fpathconf(STDOUT_FILENO, _PC_PIPE_BUF);
    if (pipe_buf != -1) {
        printf("\n💾 " YELLOW "系统 PIPE_BUF 大小: %ld 字节" RESET "\n", pipe_buf);
    }
}

int main() {
    printf(BLUE "🚀 Linux 进程间通信 - 管道通信演示程序\n" RESET);
    printf("本程序演示匿名管道和命名管道的使用方法\n");
    
    // 演示各种管道通信方式
    demo_anonymous_pipe();
    
    printf("\n" "按 Enter 键继续...");
    getchar();
    
    demo_named_pipe();
    
    printf("\n" "按 Enter 键继续...");
    getchar();
    
    demo_bidirectional_pipe();
    
    printf("\n" "按 Enter 键继续...");
    getchar();
    
    show_pipe_characteristics();
    
    printf("\n" GREEN "🎉 管道通信演示完成！" RESET "\n");
    printf("💡 " YELLOW "学习要点:" RESET "\n");
    printf("  1. 匿名管道用于父子进程通信\n");
    printf("  2. 命名管道可用于任意进程通信\n");
    printf("  3. 管道是半双工的，需要两个管道实现双向通信\n");
    printf("  4. 管道读写具有同步特性\n");
    
    return 0;
}
