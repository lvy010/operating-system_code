# C++ 智能指针实验 Makefile
# 编译器和标志设置
CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -O2 -pthread
DEBUG_FLAGS = -std=c++14 -Wall -Wextra -g -pthread -DDEBUG

# 目标文件和源文件
TARGETS = shared_ptr_demo unique_ptr_demo circular_reference_demo weak_ptr_demo thread_safety_demo
SOURCES = 01_shared_ptr_demo.cpp 02_unique_ptr_demo.cpp 03_circular_reference_demo.cpp 04_weak_ptr_demo.cpp 05_thread_safety_demo.cpp

# 默认目标
all: $(TARGETS)

# 编译规则
shared_ptr_demo: 01_shared_ptr_demo.cpp
	@echo "编译 shared_ptr 演示程序..."
	$(CXX) $(CXXFLAGS) -o $@ $<
	@echo "编译完成: $@"

unique_ptr_demo: 02_unique_ptr_demo.cpp
	@echo "编译 unique_ptr 演示程序..."
	$(CXX) $(CXXFLAGS) -o $@ $<
	@echo "编译完成: $@"

circular_reference_demo: 03_circular_reference_demo.cpp
	@echo "编译环形引用演示程序..."
	$(CXX) $(CXXFLAGS) -o $@ $<
	@echo "编译完成: $@"

weak_ptr_demo: 04_weak_ptr_demo.cpp
	@echo "编译 weak_ptr 演示程序..."
	$(CXX) $(CXXFLAGS) -o $@ $<
	@echo "编译完成: $@"

thread_safety_demo: 05_thread_safety_demo.cpp
	@echo "编译线程安全演示程序..."
	$(CXX) $(CXXFLAGS) -o $@ $<
	@echo "编译完成: $@"

# 调试版本编译
debug: CXXFLAGS = $(DEBUG_FLAGS)
debug: $(TARGETS)
	@echo "调试版本编译完成"

# 运行所有演示程序
run: all
	@echo "=========================================="
	@echo "运行所有智能指针演示程序"
	@echo "=========================================="
	@echo ""
	@echo "1. shared_ptr 演示:"
	@echo "===================="
	./shared_ptr_demo
	@echo ""
	@echo "2. unique_ptr 演示:"
	@echo "===================="
	./unique_ptr_demo
	@echo ""
	@echo "3. 环形引用演示:"
	@echo "=================="
	./circular_reference_demo
	@echo ""
	@echo "4. weak_ptr 演示:"
	@echo "=================="
	./weak_ptr_demo
	@echo ""
	@echo "5. 线程安全演示:"
	@echo "=================="
	./thread_safety_demo
	@echo ""
	@echo "所有演示程序运行完成！"

# 运行单个程序
run-shared: shared_ptr_demo
	@echo "运行 shared_ptr 演示:"
	./shared_ptr_demo

run-unique: unique_ptr_demo
	@echo "运行 unique_ptr 演示:"
	./unique_ptr_demo

run-circular: circular_reference_demo
	@echo "运行环形引用演示:"
	./circular_reference_demo

run-weak: weak_ptr_demo
	@echo "运行 weak_ptr 演示:"
	./weak_ptr_demo

run-thread: thread_safety_demo
	@echo "运行线程安全演示:"
	./thread_safety_demo

# 清理编译产物
clean:
	@echo "清理编译产物..."
	rm -f $(TARGETS)
	rm -f *.o
	rm -f *.txt  # 清理程序运行时可能创建的测试文件
	@echo "清理完成"

# 检查编译器和环境
check:
	@echo "检查编译环境..."
	@echo "编译器版本:"
	$(CXX) --version
	@echo ""
	@echo "编译标志: $(CXXFLAGS)"
	@echo ""
	@echo "源文件:"
	@ls -la *.cpp 2>/dev/null || echo "没有找到 .cpp 文件"

# 安装（可选，如果需要系统级安装）
install: all
	@echo "安装程序到 /usr/local/bin/ ..."
	sudo cp $(TARGETS) /usr/local/bin/
	@echo "安装完成"

# 卸载
uninstall:
	@echo "从 /usr/local/bin/ 卸载程序..."
	sudo rm -f $(addprefix /usr/local/bin/, $(TARGETS))
	@echo "卸载完成"

# 打包源代码
package:
	@echo "打包源代码..."
	tar -czf smart_pointer_lab.tar.gz *.cpp *.h Makefile README.md 2>/dev/null || \
	tar -czf smart_pointer_lab.tar.gz *.cpp Makefile README.md 2>/dev/null || \
	tar -czf smart_pointer_lab.tar.gz *.cpp Makefile
	@echo "打包完成: smart_pointer_lab.tar.gz"

# 代码风格检查（如果安装了 clang-format）
format:
	@if command -v clang-format >/dev/null 2>&1; then \
		echo "格式化代码..."; \
		clang-format -i *.cpp; \
		echo "代码格式化完成"; \
	else \
		echo "未找到 clang-format，跳过代码格式化"; \
	fi

# 静态分析（如果安装了 cppcheck）
analyze:
	@if command -v cppcheck >/dev/null 2>&1; then \
		echo "运行静态代码分析..."; \
		cppcheck --enable=all --std=c++14 *.cpp; \
	else \
		echo "未找到 cppcheck，跳过静态分析"; \
	fi

# 性能测试
benchmark: all
	@echo "运行性能基准测试..."
	@echo "测试 shared_ptr 性能:"
	time ./shared_ptr_demo > /dev/null
	@echo "测试 unique_ptr 性能:"
	time ./unique_ptr_demo > /dev/null
	@echo "测试线程安全性能:"
	time ./thread_safety_demo > /dev/null

# 帮助信息
help:
	@echo "C++ 智能指针实验 Makefile 帮助"
	@echo "================================"
	@echo ""
	@echo "基本命令:"
	@echo "  make          - 编译所有演示程序"
	@echo "  make all      - 同上"
	@echo "  make debug    - 编译调试版本"
	@echo "  make clean    - 清理编译产物"
	@echo "  make run      - 编译并运行所有程序"
	@echo ""
	@echo "单独编译:"
	@echo "  make shared_ptr_demo          - 编译 shared_ptr 演示"
	@echo "  make unique_ptr_demo          - 编译 unique_ptr 演示"
	@echo "  make circular_reference_demo  - 编译环形引用演示"
	@echo "  make weak_ptr_demo           - 编译 weak_ptr 演示"
	@echo "  make thread_safety_demo      - 编译线程安全演示"
	@echo ""
	@echo "单独运行:"
	@echo "  make run-shared   - 运行 shared_ptr 演示"
	@echo "  make run-unique   - 运行 unique_ptr 演示"
	@echo "  make run-circular - 运行环形引用演示"
	@echo "  make run-weak     - 运行 weak_ptr 演示"
	@echo "  make run-thread   - 运行线程安全演示"
	@echo ""
	@echo "其他命令:"
	@echo "  make check     - 检查编译环境"
	@echo "  make package   - 打包源代码"
	@echo "  make format    - 格式化代码（需要 clang-format）"
	@echo "  make analyze   - 静态代码分析（需要 cppcheck）"
	@echo "  make benchmark - 性能基准测试"
	@echo "  make install   - 系统级安装（需要 sudo）"
	@echo "  make uninstall - 卸载程序（需要 sudo）"
	@echo "  make help      - 显示此帮助信息"

# 声明伪目标
.PHONY: all debug run run-shared run-unique run-circular run-weak run-thread clean check install uninstall package format analyze benchmark help
