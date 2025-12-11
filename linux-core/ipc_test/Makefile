# Linux IPC 学习项目 Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -D_GNU_SOURCE
LDFLAGS = -lpthread

# 程序列表
PROGRAMS = pipe_demo msgqueue_demo sharedmem_demo semaphore_demo socket_demo

# 源文件
SOURCES = 1_pipe_demo.c 2_msgqueue_demo.c 3_sharedmem_demo.c 4_semaphore_demo.c 5_socket_demo.c

# 默认目标
all: $(PROGRAMS)

# 编译规则
pipe_demo: 1_pipe_demo.c
	$(CC) $(CFLAGS) -o $@ $<
	@echo "✅ $@ 编译完成"

msgqueue_demo: 2_msgqueue_demo.c
	$(CC) $(CFLAGS) -o $@ $<
	@echo "✅ $@ 编译完成"

sharedmem_demo: 3_sharedmem_demo.c
	$(CC) $(CFLAGS) -o $@ $<
	@echo "✅ $@ 编译完成"

semaphore_demo: 4_semaphore_demo.c
	$(CC) $(CFLAGS) -o $@ $<
	@echo "✅ $@ 编译完成"

socket_demo: 5_socket_demo.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<
	@echo "✅ $@ 编译完成"

# 清理目标
clean:
	rm -f $(PROGRAMS)
	@echo "🧹 清理完成"

# 重新编译
rebuild: clean all

# 安装
install: all
	sudo cp $(PROGRAMS) /usr/local/bin/
	@echo "📦 安装完成"

# 卸载
uninstall:
	sudo rm -f $(addprefix /usr/local/bin/, $(PROGRAMS))
	@echo "🗑️  卸载完成"

# 运行测试
test: all
	@echo "🧪 运行基本测试..."
	@for prog in $(PROGRAMS); do \
		echo "测试 $$prog..."; \
		timeout 5 ./$$prog < /dev/null > /dev/null 2>&1 || true; \
		echo "$$prog 测试完成"; \
	done
	@echo "✅ 所有测试完成"

# 代码风格检查
check:
	@echo "🔍 检查代码风格..."
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=all --std=c99 --suppress=missingIncludeSystem $(SOURCES); \
	else \
		echo "⚠️  cppcheck 未安装，跳过检查"; \
	fi

# 内存检查
valgrind: all
	@echo "🔍 内存检查..."
	@if command -v valgrind >/dev/null 2>&1; then \
		for prog in $(PROGRAMS); do \
			echo "检查 $$prog..."; \
			timeout 10 valgrind --leak-check=full --error-exitcode=1 ./$$prog < /dev/null > /dev/null 2>&1 || true; \
		done; \
	else \
		echo "⚠️  valgrind 未安装，跳过内存检查"; \
	fi

# 调试版本
debug: CFLAGS += -g -DDEBUG
debug: clean all
	@echo "🐛 调试版本编译完成"

# 发布版本
release: CFLAGS += -O3 -DNDEBUG
release: clean all
	@echo "🚀 发布版本编译完成"

# 显示帮助
help:
	@echo "可用的 make 目标:"
	@echo "  all      - 编译所有程序 (默认)"
	@echo "  clean    - 清理编译文件"
	@echo "  rebuild  - 重新编译"
	@echo "  install  - 安装到系统"
	@echo "  test     - 运行基本测试"
	@echo "  check    - 代码风格检查"
	@echo "  valgrind - 内存检查"
	@echo "  debug    - 编译调试版本"
	@echo "  release  - 编译发布版本"
	@echo "  help     - 显示此帮助"

# 伪目标
.PHONY: all clean rebuild install uninstall test check valgrind debug release help
