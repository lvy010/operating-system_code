# TUN 设备测试程序 Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
TARGETS = tun_test tun_router
SOURCES = tun_test.c tun_router.c network_utils.c

# 默认目标
all: $(TARGETS)

# 编译目标
tun_test: tun_test.c
	$(CC) $(CFLAGS) -o tun_test tun_test.c
	@echo "编译完成: tun_test"

tun_router: tun_router.c network_utils.c network_utils.h
	$(CC) $(CFLAGS) -o tun_router tun_router.c network_utils.c
	@echo "编译完成: tun_router"

# 清理编译生成的文件
clean:
	rm -f $(TARGETS)
	@echo "清理完成"

# 安装（复制到 /usr/local/bin）
install: $(TARGETS)
	sudo cp tun_test /usr/local/bin/
	sudo cp tun_router /usr/local/bin/
	@echo "安装完成"

# 卸载
uninstall:
	sudo rm -f /usr/local/bin/tun_test
	sudo rm -f /usr/local/bin/tun_router
	@echo "卸载完成"

# 运行基本测试
test: tun_test
	@echo "运行基本测试模式..."
	sudo ./tun_test -t

# 运行网络测试
test-network: all
	@echo "运行网络功能测试..."
	chmod +x network_test.sh
	sudo ./network_test.sh

# 设置 VPN 服务器
setup-vpn: all
	@echo "设置 VPN 服务器..."
	chmod +x setup_vpn_server.sh
	sudo ./setup_vpn_server.sh

# 运行 TUN 路由器
run-router: tun_router
	@echo "运行 TUN 路由器（按 Ctrl+C 停止）..."
	sudo ./tun_router -h

# 显示帮助
help: tun_test
	./tun_test -h

# 检查代码风格
check:
	@echo "检查代码风格..."
	cppcheck --enable=all --std=c99 $(SOURCE) 2>/dev/null || echo "cppcheck 未安装，跳过检查"

# 调试版本
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)
	@echo "调试版本编译完成"

.PHONY: all clean install uninstall test run help check debug
