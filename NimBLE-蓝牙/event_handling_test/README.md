# 事件处理测试项目

这个项目演示了三种不同的事件处理机制：

1. **优先级队列** - 紧急事件优先处理
2. **事件路由器** - 基于内容的路由
3. **分布式总线** - 跨核通信

## 文件说明

- `priority_queue_test.cpp` - 优先级队列测试
- `event_router_test.cpp` - 事件路由器测试
- `distributed_bus_test.cpp` - 分布式总线测试
- `lock_free_queue.h` - 无锁队列实现
- `Makefile` - 构建文件
- `README.md` - 项目说明

## 编译和运行

### 编译所有测试
```bash
make
```

### 运行所有测试
```bash
make run_all
```

### 运行单个测试
```bash
make run_priority    # 优先级队列测试
make run_router      # 事件路由器测试
make run_distributed # 分布式总线测试
```

### 清理编译文件
```bash
make clean
```

## 功能说明

### 1. 优先级队列（priority_queue_test.cpp）
- 使用 `std::priority_queue` 实现事件优先级处理
- 优先级数值越小越紧急
- 演示了系统崩溃、内存告警、普通日志的处理顺序

### 2. 事件路由器（event_router_test.cpp）
- 基于事件类型的内容路由
- 支持订阅/发布模式
- 不同类型的事件（error、warning、info）由不同的处理器处理

### 3. 分布式总线（distributed_bus_test.cpp）
- 使用无锁队列实现跨核通信
- 支持点对点消息和广播消息
- 模拟多核环境下的消息传递

## 预期输出

### 优先级队列测试
```
=== 优先级队列测试（紧急事件优先处理）===
事件处理顺序（按优先级从高到低）：
处理事件: 系统崩溃 (优先级: 1)
处理事件: 内存告警 (优先级: 2)
处理事件: 普通日志 (优先级: 3)
```

### 事件路由器测试
```
=== 事件路由器测试（基于内容的路由）===
错误处理器: 数据库连接失败
警告处理器: 内存使用率过高
信息处理器: 用户登录成功
错误处理器: 网络超时
```

### 分布式总线测试
```
=== 分布式总线测试（跨核通信）===
核心 0 启动
核心 1 启动
核心 1 收到来自核心 0 的消息: Hello from core 0
核心 0 收到来自核心 1 的消息: Reply from core 1
核心 0 收到来自核心 0 的消息: Broadcast from core 0
核心 1 收到来自核心 0 的消息: Broadcast from core 0
所有核心处理完成
``` 