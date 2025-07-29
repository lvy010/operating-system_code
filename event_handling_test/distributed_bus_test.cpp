#include "lock_free_queue.h"
#include <atomic>
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>
#include <functional>

struct Message {
    int sender_core;
    std::string content;
    
    Message(int sender, const std::string& msg) 
        : sender_core(sender), content(msg) {}
};

class DistributedBus {
    std::vector<LockFreeQueue<Message>> per_core_queues; // 每个核独立队列
    std::atomic<int> next_core{0};
    int num_cores;
    std::vector<std::function<void(const Message&)>> registered_handlers;

public:
    DistributedBus(int cores) : num_cores(cores) {
        per_core_queues.resize(cores);
        registered_handlers.resize(cores);
    }

    void send(int target_core, const Message& msg) {
        if (target_core >= 0 && target_core < num_cores) {
            per_core_queues[target_core].push(msg);
        }
    }

    void broadcast(const Message& msg) {
        for (int i = 0; i < num_cores; ++i) {
            per_core_queues[i].push(msg);
        }
    }

    void receive(int core_id) {
        if (core_id >= 0 && core_id < num_cores) {
            auto msg = per_core_queues[core_id].pop();
            if (msg && registered_handlers[core_id]) {
                registered_handlers[core_id](*msg);
            }
        }
    }
    
    void register_handler(int core_id, std::function<void(const Message&)> handler) {
        if (core_id >= 0 && core_id < num_cores) {
            registered_handlers[core_id] = handler;
        }
    }
};

// 模拟多核处理函数
void core_worker(int core_id, DistributedBus& bus) {
    std::cout << "核心 " << core_id << " 启动" << std::endl;
    
    // 注册消息处理器
    bus.register_handler(core_id, [core_id](const Message& msg) {
        std::cout << "核心 " << core_id << " 收到来自核心 " 
                  << msg.sender_core << " 的消息: " << msg.content << std::endl;
    });
    
    // 发送一些测试消息
    if (core_id == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        bus.send(1, Message(core_id, "Hello from core 0"));
        bus.broadcast(Message(core_id, "Broadcast from core 0"));
    }
    
    if (core_id == 1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        bus.send(0, Message(core_id, "Reply from core 1"));
    }
    
    // 处理接收到的消息
    for (int i = 0; i < 5; ++i) {
        bus.receive(core_id);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

int main() {
    std::cout << "=== 分布式总线测试（跨核通信）===" << std::endl;
    
    const int num_cores = 2;
    DistributedBus bus(num_cores);
    
    std::vector<std::thread> threads;
    
    // 创建多个核心线程
    for (int i = 0; i < num_cores; ++i) {
        threads.emplace_back(core_worker, i, std::ref(bus));
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "所有核心处理完成" << std::endl;
    return 0;
} 