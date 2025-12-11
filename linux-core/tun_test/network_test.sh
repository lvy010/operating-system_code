#!/bin/bash

# 网络功能测试脚本
# 测试 TUN 设备、路由、iptables 功能

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_LOG="/tmp/network_test.log"
TUN_DEVICE="test_tun"
TEST_IP="192.168.100.1"
TEST_NETMASK="24"
TEST_NETWORK="192.168.100.0/24"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# 测试结果统计
TESTS_TOTAL=0
TESTS_PASSED=0
TESTS_FAILED=0

# 日志函数
log() {
    echo -e "${BLUE}[测试]${NC} $1" | tee -a "$TEST_LOG"
}

success() {
    echo -e "${GREEN}[成功]${NC} $1" | tee -a "$TEST_LOG"
    ((TESTS_PASSED++))
}

failure() {
    echo -e "${RED}[失败]${NC} $1" | tee -a "$TEST_LOG"
    ((TESTS_FAILED++))
}

warning() {
    echo -e "${YELLOW}[警告]${NC} $1" | tee -a "$TEST_LOG"
}

# 运行测试并检查结果
run_test() {
    local test_name="$1"
    local test_command="$2"
    
    ((TESTS_TOTAL++))
    log "运行测试: $test_name"
    
    if eval "$test_command" >> "$TEST_LOG" 2>&1; then
        success "$test_name"
        return 0
    else
        failure "$test_name"
        return 1
    fi
}

# 检查权限
check_root() {
    if [ "$EUID" -ne 0 ]; then
        echo -e "${RED}错误:${NC} 此脚本需要 root 权限运行"
        echo "请使用: sudo $0"
        exit 1
    fi
}

# 清理函数
cleanup() {
    log "清理测试环境..."
    
    # 停止可能运行的测试进程
    pkill -f "tun_router.*$TUN_DEVICE" 2>/dev/null || true
    
    # 删除测试路由
    ip route del "$TEST_NETWORK" 2>/dev/null || true
    
    # 清理 iptables 测试规则
    iptables -t filter -D INPUT -i "$TUN_DEVICE" -j ACCEPT 2>/dev/null || true
    iptables -t filter -D OUTPUT -o "$TUN_DEVICE" -j ACCEPT 2>/dev/null || true
    iptables -t nat -D POSTROUTING -o lo -j MASQUERADE 2>/dev/null || true
    
    # 关闭测试接口
    ip link set "$TUN_DEVICE" down 2>/dev/null || true
    
    # 删除测试接口 IP
    ip addr flush dev "$TUN_DEVICE" 2>/dev/null || true
    
    log "清理完成"
}

# 信号处理
trap cleanup EXIT INT TERM

# 编译测试
test_compilation() {
    log "=== 编译测试 ==="
    
    cd "$SCRIPT_DIR"
    
    # 测试基本 TUN 程序编译
    run_test "编译 tun_test" "make clean && make"
    
    # 测试路由器程序编译
    run_test "编译 tun_router" "gcc -Wall -Wextra -std=c99 -O2 -o tun_router tun_router.c network_utils.c"
}

# TUN 设备测试
test_tun_device() {
    log "=== TUN 设备测试 ==="
    
    # 测试基本 TUN 设备创建
    run_test "TUN 设备创建" "./tun_test -d $TUN_DEVICE -t"
    
    # 测试自定义设备名称
    run_test "自定义设备名称" "./tun_test -d test_custom -t"
    
    # 检查 TUN 模块
    run_test "TUN 内核模块检查" "lsmod | grep -q tun"
    
    # 检查设备文件
    run_test "TUN 设备文件检查" "test -c /dev/net/tun"
}

# 网络接口配置测试
test_interface_config() {
    log "=== 网络接口配置测试 ==="
    
    # 创建测试 TUN 设备
    if ./tun_test -d "$TUN_DEVICE" -t > /dev/null 2>&1; then
        log "测试 TUN 设备已创建: $TUN_DEVICE"
        
        # 测试 IP 地址配置
        run_test "IP 地址配置" "ip addr add $TEST_IP/$TEST_NETMASK dev $TUN_DEVICE"
        
        # 测试接口启用
        run_test "接口启用" "ip link set $TUN_DEVICE up"
        
        # 测试 MTU 设置
        run_test "MTU 设置" "ip link set $TUN_DEVICE mtu 1400"
        
        # 验证配置
        run_test "接口配置验证" "ip addr show $TUN_DEVICE | grep -q $TEST_IP"
        
        # 测试接口状态
        run_test "接口状态检查" "ip link show $TUN_DEVICE | grep -q 'state UP'"
        
    else
        failure "无法创建测试 TUN 设备"
    fi
}

# 路由配置测试
test_routing() {
    log "=== 路由配置测试 ==="
    
    if ip link show "$TUN_DEVICE" > /dev/null 2>&1; then
        # 测试路由添加
        run_test "路由添加" "ip route add $TEST_NETWORK dev $TUN_DEVICE metric 100"
        
        # 验证路由
        run_test "路由验证" "ip route show | grep -q '$TEST_NETWORK.*$TUN_DEVICE'"
        
        # 测试路由删除
        run_test "路由删除" "ip route del $TEST_NETWORK dev $TUN_DEVICE"
        
        # 验证路由删除
        if ip route show | grep -q "$TEST_NETWORK.*$TUN_DEVICE"; then
            failure "路由删除验证"
        else
            success "路由删除验证"
        fi
        
    else
        warning "跳过路由测试 - TUN 设备不存在"
    fi
}

# iptables 功能测试
test_iptables() {
    log "=== iptables 功能测试 ==="
    
    if command -v iptables > /dev/null 2>&1; then
        # 测试基本规则添加
        run_test "iptables 规则添加" "iptables -t filter -A INPUT -i $TUN_DEVICE -j ACCEPT"
        
        # 验证规则
        run_test "iptables 规则验证" "iptables -t filter -L INPUT | grep -q $TUN_DEVICE"
        
        # 测试 NAT 规则
        run_test "NAT 规则添加" "iptables -t nat -A POSTROUTING -o lo -j MASQUERADE"
        
        # 验证 NAT 规则
        run_test "NAT 规则验证" "iptables -t nat -L POSTROUTING | grep -q MASQUERADE"
        
        # 测试规则删除
        run_test "iptables 规则删除" "iptables -t filter -D INPUT -i $TUN_DEVICE -j ACCEPT"
        
        # 测试 NAT 规则删除
        run_test "NAT 规则删除" "iptables -t nat -D POSTROUTING -o lo -j MASQUERADE"
        
    else
        warning "iptables 不可用，跳过相关测试"
    fi
}

# IP 转发测试
test_ip_forwarding() {
    log "=== IP 转发测试 ==="
    
    # 保存当前设置
    local original_forward=$(cat /proc/sys/net/ipv4/ip_forward)
    
    # 测试启用 IP 转发
    run_test "启用 IP 转发" "echo 1 > /proc/sys/net/ipv4/ip_forward"
    
    # 验证设置
    run_test "IP 转发验证" "test \$(cat /proc/sys/net/ipv4/ip_forward) -eq 1"
    
    # 恢复原设置
    echo "$original_forward" > /proc/sys/net/ipv4/ip_forward
    log "IP 转发设置已恢复"
}

# 集成功能测试
test_integrated_functionality() {
    log "=== 集成功能测试 ==="
    
    # 测试 TUN 路由器配置模式
    run_test "TUN 路由器配置模式" "./tun_router -d test_router -i 172.16.0.1 -m 24 -c"
    
    # 测试网络状态显示
    run_test "网络状态显示" "./tun_router -s"
    
    # 测试帮助功能
    run_test "帮助功能" "./tun_router -h"
}

# 性能测试
test_performance() {
    log "=== 性能测试 ==="
    
    # 创建性能测试 TUN 设备
    if ./tun_test -d perf_tun -t > /dev/null 2>&1; then
        log "性能测试 TUN 设备已创建"
        
        # 配置设备
        ip addr add 172.17.0.1/24 dev perf_tun
        ip link set perf_tun up
        
        # 简单的延迟测试
        log "测试 TUN 设备延迟..."
        if ping -c 3 -W 1 172.17.0.1 > /dev/null 2>&1; then
            success "TUN 设备延迟测试"
        else
            # 这是预期的，因为没有实际的数据处理
            log "TUN 设备延迟测试 (预期结果)"
        fi
        
        # 清理性能测试设备
        ip link set perf_tun down 2>/dev/null || true
        
    else
        warning "跳过性能测试 - 无法创建测试设备"
    fi
}

# 错误处理测试
test_error_handling() {
    log "=== 错误处理测试 ==="
    
    # 测试非 root 用户运行
    log "测试非 root 用户错误处理..."
    if sudo -u nobody ./tun_test -t 2>/dev/null; then
        failure "非 root 用户错误处理"
    else
        success "非 root 用户错误处理"
    fi
    
    # 测试无效参数
    log "测试无效参数处理..."
    if ./tun_router -x 2>/dev/null; then
        failure "无效参数错误处理"
    else
        success "无效参数错误处理"
    fi
}

# 显示测试结果
show_results() {
    echo ""
    log "=== 测试结果汇总 ==="
    echo ""
    echo "总测试数: $TESTS_TOTAL"
    echo -e "通过测试: ${GREEN}$TESTS_PASSED${NC}"
    echo -e "失败测试: ${RED}$TESTS_FAILED${NC}"
    
    if [ $TESTS_FAILED -eq 0 ]; then
        echo -e "\n${GREEN}✓ 所有测试通过！${NC}"
        echo "系统已准备好进行 TUN 设备和路由操作"
    else
        echo -e "\n${YELLOW}⚠ 有 $TESTS_FAILED 个测试失败${NC}"
        echo "请检查失败的测试并解决相关问题"
    fi
    
    echo ""
    echo "详细日志: $TEST_LOG"
    echo ""
}

# 主函数
main() {
    echo "=== TUN 设备网络功能测试 ==="
    echo "开始时间: $(date)"
    echo "测试日志: $TEST_LOG"
    echo ""
    
    # 初始化日志
    echo "TUN 网络功能测试 - $(date)" > "$TEST_LOG"
    
    check_root
    
    # 运行各项测试
    test_compilation
    test_tun_device
    test_interface_config
    test_routing
    test_iptables
    test_ip_forwarding
    test_integrated_functionality
    test_performance
    test_error_handling
    
    # 显示结果
    show_results
    
    # 返回适当的退出码
    if [ $TESTS_FAILED -eq 0 ]; then
        exit 0
    else
        exit 1
    fi
}

# 运行主函数
main "$@"
