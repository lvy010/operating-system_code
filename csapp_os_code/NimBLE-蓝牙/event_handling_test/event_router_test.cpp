#include <unordered_map>
#include <vector>
#include <functional>
#include <iostream>

struct Event {
    std::string type;
    std::string content;
};

class EventRouter {
    std::unordered_map<std::string, 
        std::vector<std::function<void(Event)>>> handlers;

public:
    void subscribe(const std::string& event_type, auto&& handler) {
        handlers[event_type].push_back(handler);
    }

    void publish(const Event& e) {
        if (handlers.count(e.type)) {
            for (auto& handler : handlers.at(e.type)) {
                handler(e); // 只触发匹配类型的处理器
            }
        }
    }
};

int main() {
    EventRouter router;
    
    // 订阅不同类型的事件处理器
    router.subscribe("error", [](const Event& e) {
        std::cout << "错误处理器: " << e.content << std::endl;
    });
    
    router.subscribe("warning", [](const Event& e) {
        std::cout << "警告处理器: " << e.content << std::endl;
    });
    
    router.subscribe("info", [](const Event& e) {
        std::cout << "信息处理器: " << e.content << std::endl;
    });
    
    std::cout << "=== 事件路由器测试（基于内容的路由）===" << std::endl;
    
    // 发布不同类型的事件
    router.publish({"error", "数据库连接失败"});
    router.publish({"warning", "内存使用率过高"});
    router.publish({"info", "用户登录成功"});
    router.publish({"error", "网络超时"});
    
    return 0;
} 