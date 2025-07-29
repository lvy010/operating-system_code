CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -pthread

# 目标程序
TARGETS = priority_queue_test event_router_test distributed_bus_test

# 默认目标
all: $(TARGETS)

# 优先级队列测试
priority_queue_test: priority_queue_test.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

# 事件路由器测试
event_router_test: event_router_test.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

# 分布式总线测试
distributed_bus_test: distributed_bus_test.cpp lock_free_queue.h
	$(CXX) $(CXXFLAGS) -o $@ distributed_bus_test.cpp

# 运行所有测试
run_all: $(TARGETS)
	@echo "=== 运行优先级队列测试 ==="
	./priority_queue_test
	@echo ""
	@echo "=== 运行事件路由器测试 ==="
	./event_router_test
	@echo ""
	@echo "=== 运行分布式总线测试 ==="
	./distributed_bus_test

# 运行单个测试
run_priority: priority_queue_test
	./priority_queue_test

run_router: event_router_test
	./event_router_test

run_distributed: distributed_bus_test
	./distributed_bus_test

# 清理
clean:
	rm -f $(TARGETS)

.PHONY: all run_all run_priority run_router run_distributed clean 