/*
 * 共享内存通信示例程序
 * 演示 System V 共享内存的使用
 * 编译: gcc -o sharedmem_demo 3_sharedmem_demo.c
 * 运行: ./sharedmem_demo
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>

// 颜色定义
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define RESET   "\033[0m"

// 共享内存大小
#define SHM_SIZE 4096

// 共享数据结构
struct shared_data {
    int counter;                // 计数器
    char message[256];          // 消息缓冲区
    int writer_pid;             // 写入者进程ID
    int reader_pid;             // 读取者进程ID
    time_t timestamp;           // 时间戳
    int data_ready;             // 数据就绪标志
    int buffer[100];            // 数据缓冲区
    int buffer_size;            // 缓冲区中的数据数量
};

// 全局变量
static int shm_id = -1;
static struct shared_data *shared_ptr = NULL;

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
void cleanup_shared_memory() {
    if (shared_ptr != NULL) {
        if (shmdt(shared_ptr) == -1) {
            print_error("分离共享内存失败");
            perror("shmdt");
        } else {
            print_info("共享内存已分离");
        }
        shared_ptr = NULL;
    }
    
    if (shm_id != -1) {
        struct shmid_ds buf;
        if (shmctl(shm_id, IPC_STAT, &buf) != -1) {
            // 只有在没有进程附加时才删除
            if (buf.shm_nattch == 0) {
                if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
                    print_error("删除共享内存失败");
                    perror("shmctl");
                } else {
                    print_info("共享内存已删除");
                }
            }
        }
    }
}

// 信号处理函数
void signal_handler(int sig) {
    printf("\n收到信号 %d，正在清理资源...\n", sig);
    cleanup_shared_memory();
    exit(0);
}

/*
 * 创建共享内存
 */
int create_shared_memory() {
    key_t key;
    
    // 生成唯一的键值
    key = ftok("/tmp", 'S');
    if (key == -1) {
        print_error("生成键值失败");
        perror("ftok");
        return -1;
    }
    
    printf("生成的键值: 0x%x\n", key);
    
    // 创建共享内存段
    shm_id = shmget(key, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0666);
    if (shm_id == -1) {
        if (errno == EEXIST) {
            print_warning("共享内存已存在，尝试获取现有段");
            shm_id = shmget(key, SHM_SIZE, 0666);
            if (shm_id == -1) {
                print_error("获取现有共享内存失败");
                perror("shmget");
                return -1;
            }
        } else {
            print_error("创建共享内存失败");
            perror("shmget");
            return -1;
        }
    }
    
    printf("共享内存ID: %d\n", shm_id);
    printf("共享内存大小: %d 字节\n", SHM_SIZE);
    
    // 附加共享内存到当前进程
    shared_ptr = (struct shared_data *)shmat(shm_id, NULL, 0);
    if (shared_ptr == (void *)-1) {
        print_error("附加共享内存失败");
        perror("shmat");
        return -1;
    }
    
    printf("共享内存地址: %p\n", (void *)shared_ptr);
    print_success("共享内存创建/获取成功");
    
    return 0;
}

/*
 * 初始化共享数据
 */
void initialize_shared_data() {
    if (shared_ptr == NULL) return;
    
    memset(shared_ptr, 0, sizeof(struct shared_data));
    shared_ptr->counter = 0;
    strcpy(shared_ptr->message, "初始化消息");
    shared_ptr->writer_pid = 0;
    shared_ptr->reader_pid = 0;
    shared_ptr->timestamp = time(NULL);
    shared_ptr->data_ready = 0;
    shared_ptr->buffer_size = 0;
    
    print_info("共享数据已初始化");
}

/*
 * 显示共享内存状态
 */
void show_shared_memory_status() {
    struct shmid_ds buf;
    
    if (shmctl(shm_id, IPC_STAT, &buf) == -1) {
        print_error("获取共享内存状态失败");
        perror("shmctl");
        return;
    }
    
    print_header("共享内存状态信息");
    printf("共享内存ID: %d\n", shm_id);
    printf("内存段大小: %lu 字节\n", buf.shm_segsz);
    printf("附加进程数: %lu\n", buf.shm_nattch);
    printf("创建者PID: %d\n", buf.shm_cpid);
    printf("最后操作PID: %d\n", buf.shm_lpid);
    printf("最后附加时间: %s", ctime(&buf.shm_atime));
    printf("最后分离时间: %s", ctime(&buf.shm_dtime));
    printf("最后修改时间: %s", ctime(&buf.shm_ctime));
    
    if (shared_ptr != NULL) {
        printf("\n" CYAN "共享数据内容:" RESET "\n");
        printf("计数器: %d\n", shared_ptr->counter);
        printf("消息: %s\n", shared_ptr->message);
        printf("写入者PID: %d\n", shared_ptr->writer_pid);
        printf("读取者PID: %d\n", shared_ptr->reader_pid);
        printf("时间戳: %s", ctime(&shared_ptr->timestamp));
        printf("数据就绪: %s\n", shared_ptr->data_ready ? "是" : "否");
        printf("缓冲区数据量: %d\n", shared_ptr->buffer_size);
    }
}

/*
 * 演示基本的共享内存读写
 */
void demo_basic_shared_memory() {
    print_header("基本共享内存读写演示");
    
    initialize_shared_data();
    
    pid_t pid = fork();
    
    if (pid == -1) {
        print_error("创建子进程失败");
        perror("fork");
        return;
    }
    
    if (pid == 0) {
        // 子进程 - 写入数据
        print_info("子进程: 写入共享内存");
        
        for (int i = 0; i < 5; i++) {
            shared_ptr->counter = i + 1;
            snprintf(shared_ptr->message, sizeof(shared_ptr->message), 
                    "来自子进程的消息 #%d", i + 1);
            shared_ptr->writer_pid = getpid();
            shared_ptr->timestamp = time(NULL);
            shared_ptr->data_ready = 1;
            
            printf(GREEN "子进程写入: 计数器=%d, 消息=%s" RESET "\n", 
                   shared_ptr->counter, shared_ptr->message);
            
            sleep(1);
        }
        
        print_success("子进程写入完成");
        exit(0);
        
    } else {
        // 父进程 - 读取数据
        print_info("父进程: 读取共享内存");
        
        int last_counter = 0;
        for (int i = 0; i < 5; i++) {
            // 等待数据更新
            while (shared_ptr->counter == last_counter) {
                usleep(100000);  // 100ms
            }
            
            shared_ptr->reader_pid = getpid();
            last_counter = shared_ptr->counter;
            
            printf(CYAN "父进程读取: 计数器=%d, 消息=%s, 写入者PID=%d" RESET "\n",
                   shared_ptr->counter, shared_ptr->message, shared_ptr->writer_pid);
        }
        
        // 等待子进程结束
        int status;
        wait(&status);
        print_success("基本共享内存通信完成");
    }
}

/*
 * 演示生产者消费者模式
 */
void demo_producer_consumer() {
    print_header("生产者消费者模式演示");
    
    // 重置共享数据
    shared_ptr->buffer_size = 0;
    shared_ptr->data_ready = 0;
    
    pid_t producer_pid = fork();
    
    if (producer_pid == -1) {
        print_error("创建生产者进程失败");
        return;
    }
    
    if (producer_pid == 0) {
        // 生产者进程
        print_info("生产者: 开始生产数据");
        
        for (int i = 0; i < 10; i++) {
            // 等待缓冲区有空间
            while (shared_ptr->buffer_size >= 100) {
                usleep(10000);  // 10ms
            }
            
            // 生产数据
            int data = rand() % 1000;
            shared_ptr->buffer[shared_ptr->buffer_size] = data;
            shared_ptr->buffer_size++;
            shared_ptr->timestamp = time(NULL);
            
            printf(GREEN "生产者生产: 数据=%d, 缓冲区大小=%d" RESET "\n", 
                   data, shared_ptr->buffer_size);
            
            usleep(200000);  // 200ms
        }
        
        print_success("生产者完成");
        exit(0);
    }
    
    pid_t consumer_pid = fork();
    
    if (consumer_pid == -1) {
        print_error("创建消费者进程失败");
        kill(producer_pid, SIGTERM);
        return;
    }
    
    if (consumer_pid == 0) {
        // 消费者进程
        print_info("消费者: 开始消费数据");
        
        int consumed = 0;
        while (consumed < 10) {
            // 等待缓冲区有数据
            while (shared_ptr->buffer_size <= 0) {
                usleep(10000);  // 10ms
            }
            
            // 消费数据
            int data = shared_ptr->buffer[0];
            
            // 移动数据 (简单的队列实现)
            for (int i = 0; i < shared_ptr->buffer_size - 1; i++) {
                shared_ptr->buffer[i] = shared_ptr->buffer[i + 1];
            }
            shared_ptr->buffer_size--;
            consumed++;
            
            printf(CYAN "消费者消费: 数据=%d, 缓冲区大小=%d" RESET "\n", 
                   data, shared_ptr->buffer_size);
            
            usleep(300000);  // 300ms
        }
        
        print_success("消费者完成");
        exit(0);
    }
    
    // 父进程等待两个子进程
    int status;
    waitpid(producer_pid, &status, 0);
    waitpid(consumer_pid, &status, 0);
    
    print_success("生产者消费者演示完成");
}

/*
 * 演示多进程并发访问
 */
void demo_concurrent_access() {
    print_header("多进程并发访问演示");
    
    const int num_processes = 4;
    pid_t pids[num_processes];
    
    // 重置计数器
    shared_ptr->counter = 0;
    
    // 创建多个子进程
    for (int i = 0; i < num_processes; i++) {
        pids[i] = fork();
        
        if (pids[i] == -1) {
            print_error("创建子进程失败");
            continue;
        }
        
        if (pids[i] == 0) {
            // 子进程 - 并发修改计数器
            printf(YELLOW "进程 %d (PID:%d) 开始工作" RESET "\n", i+1, getpid());
            
            for (int j = 0; j < 5; j++) {
                // 读取当前值
                int current = shared_ptr->counter;
                
                // 模拟一些处理时间
                usleep(10000 + (rand() % 20000));  // 10-30ms
                
                // 写入新值 (这里会有竞争条件)
                shared_ptr->counter = current + 1;
                
                printf("进程 %d: 计数器 %d -> %d\n", i+1, current, shared_ptr->counter);
                
                usleep(50000);  // 50ms
            }
            
            printf(YELLOW "进程 %d 完成工作" RESET "\n", i+1);
            exit(0);
        }
    }
    
    // 等待所有子进程结束
    for (int i = 0; i < num_processes; i++) {
        if (pids[i] > 0) {
            int status;
            waitpid(pids[i], &status, 0);
        }
    }
    
    printf(MAGENTA "最终计数器值: %d (期望值: %d)" RESET "\n", 
           shared_ptr->counter, num_processes * 5);
    
    if (shared_ptr->counter != num_processes * 5) {
        print_warning("发生了竞争条件！这说明需要同步机制");
    }
    
    print_success("并发访问演示完成");
}

/*
 * 演示内存映射和性能测试
 */
void demo_performance_test() {
    print_header("共享内存性能测试");
    
    const int test_size = 1000000;  // 100万次操作
    clock_t start, end;
    
    print_info("开始性能测试...");
    printf("测试规模: %d 次写入操作\n", test_size);
    
    pid_t pid = fork();
    
    if (pid == -1) {
        print_error("创建子进程失败");
        return;
    }
    
    if (pid == 0) {
        // 子进程 - 大量写入
        start = clock();
        
        for (int i = 0; i < test_size; i++) {
            shared_ptr->counter = i;
            if (i % 100000 == 0) {
                printf("进度: %d%%\n", (i * 100) / test_size);
            }
        }
        
        end = clock();
        double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
        
        printf(GREEN "写入完成: %d 次操作耗时 %.2f 秒" RESET "\n", test_size, cpu_time);
        printf("平均速度: %.0f 次/秒\n", test_size / cpu_time);
        
        exit(0);
        
    } else {
        // 父进程等待
        int status;
        wait(&status);
        
        printf("最终计数器值: %d\n", shared_ptr->counter);
        print_success("性能测试完成");
    }
}

/*
 * 显示共享内存特性和限制
 */
void show_shared_memory_characteristics() {
    print_header("共享内存特性和限制");
    
    printf("📋 " YELLOW "共享内存特性:" RESET "\n");
    printf("  • 最快的进程间通信方式\n");
    printf("  • 多个进程直接访问同一块内存\n");
    printf("  • 不需要数据拷贝，直接内存访问\n");
    printf("  • 适合大量数据共享\n");
    printf("  • 内存映射到进程地址空间\n");
    
    printf("\n📏 " YELLOW "共享内存限制:" RESET "\n");
    printf("  • 需要同步机制防止竞争条件\n");
    printf("  • 没有内置的同步和互斥机制\n");
    printf("  • 需要显式删除，否则会持续存在\n");
    printf("  • 受系统内存大小限制\n");
    printf("  • 需要考虑字节序和对齐问题\n");
    
    printf("\n⚠️  " YELLOW "注意事项:" RESET "\n");
    printf("  • 必须配合信号量或互斥锁使用\n");
    printf("  • 进程崩溃可能导致数据不一致\n");
    printf("  • 需要处理内存访问冲突\n");
    printf("  • 适合读多写少的场景\n");
    
    // 显示系统限制
    printf("\n💾 " YELLOW "系统限制查看:" RESET "\n");
    printf("  • 最大段大小: cat /proc/sys/kernel/shmmax\n");
    printf("  • 最大段数量: cat /proc/sys/kernel/shmmni\n");
    printf("  • 总共享内存: cat /proc/sys/kernel/shmall\n");
}

int main() {
    printf(BLUE "🚀 Linux 进程间通信 - 共享内存演示程序\n" RESET);
    printf("本程序演示 System V 共享内存的各种使用方法\n");
    
    // 设置随机种子
    srand(time(NULL));
    
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // 创建共享内存
    if (create_shared_memory() == -1) {
        return 1;
    }
    
    // 演示各种共享内存操作
    demo_basic_shared_memory();
    
    printf("\n按 Enter 键继续...");
    getchar();
    
    demo_producer_consumer();
    
    printf("\n按 Enter 键继续...");
    getchar();
    
    demo_concurrent_access();
    
    printf("\n按 Enter 键继续...");
    getchar();
    
    demo_performance_test();
    
    printf("\n按 Enter 键继续...");
    getchar();
    
    show_shared_memory_status();
    
    printf("\n按 Enter 键继续...");
    getchar();
    
    show_shared_memory_characteristics();
    
    // 清理资源
    cleanup_shared_memory();
    
    printf("\n" GREEN "🎉 共享内存演示完成！" RESET "\n");
    printf("💡 " YELLOW "学习要点:" RESET "\n");
    printf("  1. 共享内存是最快的IPC方式\n");
    printf("  2. 需要配合同步机制使用\n");
    printf("  3. 适合大量数据的高速共享\n");
    printf("  4. 注意竞争条件和数据一致性\n");
    
    return 0;
}
