#include <queue>
#include <iostream>

struct Event 
{
    int priority; // 优先级数值越小越紧急
    std::string data;
    
    bool operator<(const Event& e) const 
    { 
        return priority > e.priority; // 重载运算符实现小顶堆
    }
};

int main() {
    std::priority_queue<Event> pq;
    pq.push({1, "系统崩溃"}); // 最高优先级
    pq.push({3, "普通日志"});
    pq.push({2, "内存告警"});

    std::cout << "=== 优先级队列测试（紧急事件优先处理）===" << std::endl;
    std::cout << "事件处理顺序（按优先级从高到低）：" << std::endl;
    
    while (!pq.empty()) {
        auto e = pq.top();
        std::cout << "处理事件: " << e.data << " (优先级: " << e.priority << ")" << std::endl;
        pq.pop();
    }
    
    return 0;
} 