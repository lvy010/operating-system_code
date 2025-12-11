/*
 * 信号量通信示例程序
 * 演示 System V 信号量的使用，用于进程同步与互斥
 * 编译: gcc -o semaphore_demo 4_semaphore_demo.c
 * 运行: ./semaphore_demo
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

// 颜色定义
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define RESET   "\033[0m"

// 信号量操作宏
#define SEM_MUTEX   0   // 互斥信号量
#define SEM_FULL    1   // 满缓冲区信号量
#define SEM_EMPTY   2   // 空缓冲区信号量

// 共享内存大小
#define SHM_SIZE 4096
#define BUFFER_SIZE 10

// 共享数据结构
struct shared_buffer {
    int buffer[BUFFER_SIZE];    // 环形缓冲区
    int in;                     // 写入位置
    int out;                    // 读取位置
    int count;                  // 当前数据数量
    int total_produced;         // 总生产数量
    int total_consumed;         // 总消费数量
    time_t last_update;         // 最后更新时间
};

// 全局变量
static int sem_id = -1;
static int shm_id = -1;
static struct shared_buffer *shared_ptr = NULL;

// System V 信号量的 union semun 在某些系统上需要手动定义
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
    int val;                    /* value for SETVAL */
    struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
    unsigned short *array;      /* array for GETALL, SETALL */
    struct seminfo *__buf;      /* buffer for IPC_INFO */
};
#endif

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
void cleanup_resources() {
    if (shared_ptr != NULL) {
        if (shmdt(shared_ptr) == -1) {
            print_error("分离共享内存失败");
        } else {
            print_info("共享内存已分离");
        }
        shared_ptr = NULL;
    }
    
    if (shm_id != -1) {
        struct shmid_ds buf;
        if (shmctl(shm_id, IPC_STAT, &buf) != -1 && buf.shm_nattch == 0) {
            if (shmctl(shm_id, IPC_RMID, NULL) != -1) {
                print_info("共享内存已删除");
            }
        }
    }
    
    if (sem_id != -1) {
        if (semctl(sem_id, 0, IPC_RMID) != -1) {
            print_info("信号量集已删除");
        }
    }
}

// 信号处理函数
void signal_handler(int sig) {
    printf("\n收到信号 %d，正在清理资源...\n", sig);
    cleanup_resources();
    exit(0);
}

/*
 * 信号量P操作 (wait/down)
 */
int sem_p(int sem_id, int sem_num) {
    struct sembuf sb;
    sb.sem_num = sem_num;
    sb.sem_op = -1;         // P操作，减1
    sb.sem_flg = SEM_UNDO;  // 进程结束时自动撤销操作
    
    if (semop(sem_id, &sb, 1) == -1) {
        print_error("P操作失败");
        perror("semop");
        return -1;
    }
    return 0;
}

/*
 * 信号量V操作 (signal/up)
 */
int sem_v(int sem_id, int sem_num) {
    struct sembuf sb;
    sb.sem_num = sem_num;
    sb.sem_op = 1;          // V操作，加1
    sb.sem_flg = SEM_UNDO;  // 进程结束时自动撤销操作
    
    if (semop(sem_id, &sb, 1) == -1) {
        print_error("V操作失败");
        perror("semop");
        return -1;
    }
    return 0;
}

/*
 * 获取信号量值
 */
int get_sem_value(int sem_id, int sem_num) {
    int val = semctl(sem_id, sem_num, GETVAL);
    if (val == -1) {
        print_error("获取信号量值失败");
        perror("semctl");
    }
    return val;
}

/*
 * 创建信号量集
 */
int create_semaphore_set() {
    key_t key;
    union semun arg;
    
    // 生成唯一的键值
    key = ftok("/tmp", 'E');
    if (key == -1) {
        print_error("生成信号量键值失败");
        perror("ftok");
        return -1;
    }
    
    printf("信号量键值: 0x%x\n", key);
    
    // 创建信号量集 (3个信号量)
    sem_id = semget(key, 3, IPC_CREAT | IPC_EXCL | 0666);
    if (sem_id == -1) {
        if (errno == EEXIST) {
            print_warning("信号量集已存在，尝试获取现有集合");
            sem_id = semget(key, 3, 0666);
            if (sem_id == -1) {
                print_error("获取现有信号量集失败");
                perror("semget");
                return -1;
            }
        } else {
            print_error("创建信号量集失败");
            perror("semget");
            return -1;
        }
    }
    
    printf("信号量集ID: %d\n", sem_id);
    
    // 初始化信号量值
    arg.val = 1;
    if (semctl(sem_id, SEM_MUTEX, SETVAL, arg) == -1) {
        print_error("初始化互斥信号量失败");
        perror("semctl");
        return -1;
    }
    
    arg.val = 0;
    if (semctl(sem_id, SEM_FULL, SETVAL, arg) == -1) {
        print_error("初始化满信号量失败");
        perror("semctl");
        return -1;
    }
    
    arg.val = BUFFER_SIZE;
    if (semctl(sem_id, SEM_EMPTY, SETVAL, arg) == -1) {
        print_error("初始化空信号量失败");
        perror("semctl");
        return -1;
    }
    
    print_success("信号量集创建和初始化完成");
    printf("  • 互斥信号量 (SEM_MUTEX): %d\n", get_sem_value(sem_id, SEM_MUTEX));
    printf("  • 满信号量 (SEM_FULL): %d\n", get_sem_value(sem_id, SEM_FULL));
    printf("  • 空信号量 (SEM_EMPTY): %d\n", get_sem_value(sem_id, SEM_EMPTY));
    
    return 0;
}

/*
 * 创建共享内存
 */
int create_shared_memory() {
    key_t key;
    
    // 生成键值
    key = ftok("/tmp", 'S');
    if (key == -1) {
        print_error("生成共享内存键值失败");
        perror("ftok");
        return -1;
    }
    
    // 创建共享内存
    shm_id = shmget(key, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0666);
    if (shm_id == -1) {
        if (errno == EEXIST) {
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
    
    // 附加共享内存
    shared_ptr = (struct shared_buffer *)shmat(shm_id, NULL, 0);
    if (shared_ptr == (void *)-1) {
        print_error("附加共享内存失败");
        perror("shmat");
        return -1;
    }
    
    // 初始化共享数据
    memset(shared_ptr, 0, sizeof(struct shared_buffer));
    shared_ptr->in = 0;
    shared_ptr->out = 0;
    shared_ptr->count = 0;
    shared_ptr->total_produced = 0;
    shared_ptr->total_consumed = 0;
    shared_ptr->last_update = time(NULL);
    
    print_success("共享内存创建和初始化完成");
    return 0;
}

/*
 * 显示信号量状态
 */
void show_semaphore_status() {
    struct semid_ds buf;
    
    if (semctl(sem_id, 0, IPC_STAT, &buf) == -1) {
        print_error("获取信号量状态失败");
        perror("semctl");
        return;
    }
    
    print_header("信号量集状态信息");
    printf("信号量集ID: %d\n", sem_id);
    printf("信号量数量: %lu\n", buf.sem_nsems);
    printf("创建者PID: %d\n", buf.sem_perm.cuid);
    printf("最后操作时间: %s", ctime(&buf.sem_otime));
    printf("最后修改时间: %s", ctime(&buf.sem_ctime));
    
    printf("\n" CYAN "各信号量当前值:" RESET "\n");
    printf("  • 互斥信号量 (SEM_MUTEX): %d\n", get_sem_value(sem_id, SEM_MUTEX));
    printf("  • 满信号量 (SEM_FULL): %d\n", get_sem_value(sem_id, SEM_FULL));
    printf("  • 空信号量 (SEM_EMPTY): %d\n", get_sem_value(sem_id, SEM_EMPTY));
    
    if (shared_ptr != NULL) {
        printf("\n" CYAN "共享缓冲区状态:" RESET "\n");
        printf("  • 当前数据数量: %d\n", shared_ptr->count);
        printf("  • 写入位置: %d\n", shared_ptr->in);
        printf("  • 读取位置: %d\n", shared_ptr->out);
        printf("  • 总生产数量: %d\n", shared_ptr->total_produced);
        printf("  • 总消费数量: %d\n", shared_ptr->total_consumed);
    }
}

/*
 * 生产者函数
 */
void producer_process(int producer_id, int items_to_produce) {
    printf(GREEN "生产者 %d (PID:%d) 开始工作" RESET "\n", producer_id, getpid());
    
    for (int i = 0; i < items_to_produce; i++) {
        int item = producer_id * 1000 + i + 1;
        
        // P(empty) - 等待空槽位
        printf("生产者 %d: 等待空槽位...\n", producer_id);
        if (sem_p(sem_id, SEM_EMPTY) == -1) {
            print_error("生产者等待空槽位失败");
            break;
        }
        
        // P(mutex) - 获取互斥锁
        printf("生产者 %d: 获取互斥锁...\n", producer_id);
        if (sem_p(sem_id, SEM_MUTEX) == -1) {
            print_error("生产者获取互斥锁失败");
            break;
        }
        
        // 临界区 - 生产数据
        shared_ptr->buffer[shared_ptr->in] = item;
        printf(GREEN "生产者 %d: 生产物品 %d 到位置 %d" RESET "\n", 
               producer_id, item, shared_ptr->in);
        
        shared_ptr->in = (shared_ptr->in + 1) % BUFFER_SIZE;
        shared_ptr->count++;
        shared_ptr->total_produced++;
        shared_ptr->last_update = time(NULL);
        
        // V(mutex) - 释放互斥锁
        if (sem_v(sem_id, SEM_MUTEX) == -1) {
            print_error("生产者释放互斥锁失败");
            break;
        }
        
        // V(full) - 增加满槽位
        if (sem_v(sem_id, SEM_FULL) == -1) {
            print_error("生产者增加满槽位失败");
            break;
        }
        
        // 模拟生产时间
        usleep(200000 + (rand() % 300000));  // 200-500ms
    }
    
    printf(GREEN "生产者 %d 完成工作" RESET "\n", producer_id);
}

/*
 * 消费者函数
 */
void consumer_process(int consumer_id, int items_to_consume) {
    printf(CYAN "消费者 %d (PID:%d) 开始工作" RESET "\n", consumer_id, getpid());
    
    for (int i = 0; i < items_to_consume; i++) {
        // P(full) - 等待满槽位
        printf("消费者 %d: 等待满槽位...\n", consumer_id);
        if (sem_p(sem_id, SEM_FULL) == -1) {
            print_error("消费者等待满槽位失败");
            break;
        }
        
        // P(mutex) - 获取互斥锁
        printf("消费者 %d: 获取互斥锁...\n", consumer_id);
        if (sem_p(sem_id, SEM_MUTEX) == -1) {
            print_error("消费者获取互斥锁失败");
            break;
        }
        
        // 临界区 - 消费数据
        int item = shared_ptr->buffer[shared_ptr->out];
        printf(CYAN "消费者 %d: 消费物品 %d 从位置 %d" RESET "\n", 
               consumer_id, item, shared_ptr->out);
        
        shared_ptr->out = (shared_ptr->out + 1) % BUFFER_SIZE;
        shared_ptr->count--;
        shared_ptr->total_consumed++;
        shared_ptr->last_update = time(NULL);
        
        // V(mutex) - 释放互斥锁
        if (sem_v(sem_id, SEM_MUTEX) == -1) {
            print_error("消费者释放互斥锁失败");
            break;
        }
        
        // V(empty) - 增加空槽位
        if (sem_v(sem_id, SEM_EMPTY) == -1) {
            print_error("消费者增加空槽位失败");
            break;
        }
        
        // 模拟消费时间
        usleep(300000 + (rand() % 400000));  // 300-700ms
    }
    
    printf(CYAN "消费者 %d 完成工作" RESET "\n", consumer_id);
}

/*
 * 演示基本的信号量互斥
 */
void demo_basic_mutex() {
    print_header("基本信号量互斥演示");
    
    const int num_processes = 3;
    pid_t pids[num_processes];
    
    for (int i = 0; i < num_processes; i++) {
        pids[i] = fork();
        
        if (pids[i] == -1) {
            print_error("创建子进程失败");
            continue;
        }
        
        if (pids[i] == 0) {
            // 子进程 - 访问临界区
            printf(YELLOW "进程 %d (PID:%d) 尝试进入临界区" RESET "\n", i+1, getpid());
            
            // P(mutex) - 进入临界区
            if (sem_p(sem_id, SEM_MUTEX) == -1) {
                exit(1);
            }
            
            printf(GREEN "进程 %d 进入临界区" RESET "\n", i+1);
            
            // 模拟临界区操作
            for (int j = 0; j < 3; j++) {
                printf("进程 %d: 临界区操作 %d/3\n", i+1, j+1);
                sleep(1);
            }
            
            printf(GREEN "进程 %d 离开临界区" RESET "\n", i+1);
            
            // V(mutex) - 离开临界区
            if (sem_v(sem_id, SEM_MUTEX) == -1) {
                exit(1);
            }
            
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
    
    print_success("基本互斥演示完成");
}

/*
 * 演示生产者消费者问题
 */
void demo_producer_consumer() {
    print_header("生产者消费者问题演示");
    
    const int num_producers = 2;
    const int num_consumers = 2;
    const int items_per_producer = 5;
    const int items_per_consumer = 5;
    
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
            producer_process(i + 1, items_per_producer);
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
            consumer_process(i + 1, items_per_consumer);
            exit(0);
        }
    }
    
    // 定期显示状态
    for (int j = 0; j < 10; j++) {
        sleep(2);
        printf("\n" MAGENTA "=== 状态更新 %d ===" RESET "\n", j + 1);
        printf("生产总数: %d, 消费总数: %d, 缓冲区: %d/%d\n",
               shared_ptr->total_produced, shared_ptr->total_consumed,
               shared_ptr->count, BUFFER_SIZE);
        printf("信号量状态 - 互斥:%d, 满:%d, 空:%d\n",
               get_sem_value(sem_id, SEM_MUTEX),
               get_sem_value(sem_id, SEM_FULL),
               get_sem_value(sem_id, SEM_EMPTY));
    }
    
    // 等待所有子进程结束
    for (i = 0; i < num_producers + num_consumers; i++) {
        if (pids[i] > 0) {
            int status;
            waitpid(pids[i], &status, 0);
        }
    }
    
    print_success("生产者消费者演示完成");
}

/*
 * 显示信号量特性和使用场景
 */
void show_semaphore_characteristics() {
    print_header("信号量特性和使用场景");
    
    printf("📋 " YELLOW "信号量特性:" RESET "\n");
    printf("  • 用于进程间同步和互斥\n");
    printf("  • 支持原子操作 (P/V 操作)\n");
    printf("  • 可以控制资源访问数量\n");
    printf("  • 支持 SEM_UNDO 操作撤销\n");
    printf("  • 可以创建信号量集合\n");
    
    printf("\n🔧 " YELLOW "主要用途:" RESET "\n");
    printf("  • 互斥锁 (值为1的信号量)\n");
    printf("  • 资源计数 (值为N的信号量)\n");
    printf("  • 进程同步 (事件通知)\n");
    printf("  • 生产者消费者问题\n");
    printf("  • 读者写者问题\n");
    
    printf("\n⚠️  " YELLOW "注意事项:" RESET "\n");
    printf("  • 避免死锁 (获取锁的顺序)\n");
    printf("  • 使用 SEM_UNDO 防止进程崩溃\n");
    printf("  • 及时释放信号量\n");
    printf("  • 注意信号量的初始值设置\n");
    
    printf("\n🆚 " YELLOW "与其他IPC比较:" RESET "\n");
    printf("  • vs 互斥锁: 更重量级，跨进程\n");
    printf("  • vs 共享内存: 提供同步机制\n");
    printf("  • vs 消息队列: 专注于同步而非数据传输\n");
}

int main() {
    printf(BLUE "🚀 Linux 进程间通信 - 信号量演示程序\n" RESET);
    printf("本程序演示 System V 信号量的同步和互斥功能\n");
    
    // 设置随机种子
    srand(time(NULL));
    
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // 创建信号量集
    if (create_semaphore_set() == -1) {
        return 1;
    }
    
    // 创建共享内存 (用于生产者消费者演示)
    if (create_shared_memory() == -1) {
        cleanup_resources();
        return 1;
    }
    
    // 演示各种信号量操作
    demo_basic_mutex();
    
    printf("\n按 Enter 键继续...");
    getchar();
    
    demo_producer_consumer();
    
    printf("\n按 Enter 键继续...");
    getchar();
    
    show_semaphore_status();
    
    printf("\n按 Enter 键继续...");
    getchar();
    
    show_semaphore_characteristics();
    
    // 清理资源
    cleanup_resources();
    
    printf("\n" GREEN "🎉 信号量演示完成！" RESET "\n");
    printf("💡 " YELLOW "学习要点:" RESET "\n");
    printf("  1. 信号量是进程同步的重要工具\n");
    printf("  2. P/V 操作是原子性的\n");
    printf("  3. 可以解决生产者消费者等经典问题\n");
    printf("  4. 需要合理设计避免死锁\n");
    
    return 0;
}
