#!/bin/bash

# 网络配置清理脚本
# 安全地清理所有测试相关的网络配置，不影响原有网络

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# 日志函数
log() {
    echo -e "${BLUE}[清理]${NC} $1"
}

success() {
    echo -e "${GREEN}[完成]${NC} $1"
}

warning() {
    echo -e "${YELLOW}[警告]${NC} $1"
}

error() {
    echo -e "${RED}[错误]${NC} $1"
}

# 检查权限
check_root() {
    if [ "$EUID" -ne 0 ]; then
        error "此脚本需要 root 权限运行"
        echo "请使用: sudo $0"
        exit 1
    fi
}

# 备份当前网络状态
backup_current_state() {
    log "备份当前网络状态..."
    
    local backup_dir="/tmp/network_backup_$(date +%Y%m%d_%H%M%S)"
    mkdir -p "$backup_dir"
    
    # 备份网络接口信息
    ip addr show > "$backup_dir/interfaces.txt" 2>/dev/null || true
    ip route show > "$backup_dir/routes.txt" 2>/dev/null || true
    iptables -t filter -L -n > "$backup_dir/iptables_filter.txt" 2>/dev/null || true
    iptables -t nat -L -n > "$backup_dir/iptables_nat.txt" 2>/dev/null || true
    cat /proc/sys/net/ipv4/ip_forward > "$backup_dir/ip_forward.txt" 2>/dev/null || true
    
    echo "$backup_dir" > /tmp/last_network_backup
    log "网络状态已备份到: $backup_dir"
}

# 停止所有测试进程
stop_test_processes() {
    log "停止所有测试相关进程..."
    
    # 停止 tun_test 进程
    if pgrep -f "tun_test" > /dev/null; then
        log "停止 tun_test 进程..."
        pkill -TERM -f "tun_test" || true
        sleep 2
        pkill -KILL -f "tun_test" 2>/dev/null || true
    fi
    
    # 停止 tun_router 进程
    if pgrep -f "tun_router" > /dev/null; then
        log "停止 tun_router 进程..."
        pkill -TERM -f "tun_router" || true
        sleep 2
        pkill -KILL -f "tun_router" 2>/dev/null || true
    fi
    
    success "测试进程已停止"
}

# 清理测试 TUN 设备
cleanup_tun_devices() {
    log "清理测试 TUN 设备..."
    
    # 定义可能的测试设备名称
    local test_devices=(
        "tun0" "tun1" "tun2"
        "test_tun" "test_router" "test_custom"
        "perf_tun" "mytun" "vpn_tun"
    )
    
    for device in "${test_devices[@]}"; do
        if ip link show "$device" > /dev/null 2>&1; then
            log "清理设备: $device"
            
            # 删除设备上的路由
            ip route show | grep "dev $device" | while read route; do
                log "删除路由: $route"
                ip route del $route 2>/dev/null || true
            done
            
            # 清除设备 IP 地址
            ip addr flush dev "$device" 2>/dev/null || true
            
            # 关闭设备
            ip link set "$device" down 2>/dev/null || true
            
            success "设备 $device 已清理"
        fi
    done
}

# 清理测试路由
cleanup_test_routes() {
    log "清理测试路由..."
    
    # 定义测试网络段
    local test_networks=(
        "10.0.0.0/24"
        "192.168.10.0/24"
        "192.168.100.0/24"
        "172.16.0.0/24"
        "172.17.0.0/24"
    )
    
    for network in "${test_networks[@]}"; do
        if ip route show | grep -q "$network"; then
            log "删除测试网络路由: $network"
            ip route del "$network" 2>/dev/null || true
        fi
    done
    
    success "测试路由已清理"
}

# 清理测试 iptables 规则
cleanup_test_iptables() {
    log "清理测试 iptables 规则..."
    
    # 清理可能的测试规则
    local test_interfaces=("tun0" "tun1" "test_tun" "test_router" "perf_tun")
    
    for iface in "${test_interfaces[@]}"; do
        # 清理 filter 表规则
        iptables -t filter -D INPUT -i "$iface" -j ACCEPT 2>/dev/null || true
        iptables -t filter -D OUTPUT -o "$iface" -j ACCEPT 2>/dev/null || true
        iptables -t filter -D FORWARD -i "$iface" -j ACCEPT 2>/dev/null || true
        iptables -t filter -D FORWARD -o "$iface" -j ACCEPT 2>/dev/null || true
        
        # 清理 NAT 表规则
        iptables -t nat -D POSTROUTING -o "$iface" -j MASQUERADE 2>/dev/null || true
        iptables -t nat -D PREROUTING -i "$iface" -j ACCEPT 2>/dev/null || true
    done
    
    # 清理环回接口上的测试规则
    iptables -t nat -D POSTROUTING -o lo -j MASQUERADE 2>/dev/null || true
    
    success "测试 iptables 规则已清理"
}

# 还原 IP 转发设置
restore_ip_forwarding() {
    log "检查 IP 转发设置..."
    
    # 检查是否有备份的 IP 转发设置
    local backup_file
    if [ -f "/tmp/last_network_backup" ]; then
        backup_file="$(cat /tmp/last_network_backup)/ip_forward.txt"
        if [ -f "$backup_file" ]; then
            local original_forward=$(cat "$backup_file")
            log "还原 IP 转发设置为: $original_forward"
            echo "$original_forward" > /proc/sys/net/ipv4/ip_forward
            success "IP 转发设置已还原"
            return
        fi
    fi
    
    # 如果没有备份，设置为保守的默认值（通常是 0）
    warning "没有找到原始 IP 转发设置备份，设置为默认值 0"
    echo "0" > /proc/sys/net/ipv4/ip_forward
}

# 清理临时文件
cleanup_temp_files() {
    log "清理临时文件..."
    
    # 清理测试日志
    rm -f /tmp/network_test.log
    rm -f /var/log/tun_vpn.log
    
    # 清理可能的配置文件
    rm -f "$PWD/vpn_server.conf" 2>/dev/null || true
    
    success "临时文件已清理"
}

# 验证清理结果
verify_cleanup() {
    log "验证清理结果..."
    
    local issues_found=0
    
    # 检查是否还有测试进程运行
    if pgrep -f "tun_test\|tun_router" > /dev/null; then
        warning "仍有测试进程在运行"
        ((issues_found++))
    fi
    
    # 检查是否还有测试 TUN 设备
    local remaining_devices=()
    for device in tun0 tun1 test_tun test_router perf_tun; do
        if ip link show "$device" > /dev/null 2>&1; then
            remaining_devices+=("$device")
        fi
    done
    
    if [ ${#remaining_devices[@]} -gt 0 ]; then
        warning "仍有测试设备存在: ${remaining_devices[*]}"
        ((issues_found++))
    fi
    
    # 检查是否还有测试路由
    local test_routes=$(ip route show | grep -E "10\.0\.0\.0/24|192\.168\.10\.0/24|192\.168\.100\.0/24" || true)
    if [ -n "$test_routes" ]; then
        warning "仍有测试路由存在"
        echo "$test_routes"
        ((issues_found++))
    fi
    
    if [ $issues_found -eq 0 ]; then
        success "清理验证通过，网络环境已恢复"
    else
        warning "发现 $issues_found 个问题，可能需要手动处理"
    fi
}

# 显示当前网络状态
show_current_status() {
    log "当前网络状态:"
    echo ""
    echo "=== 网络接口 ==="
    ip addr show | grep -E "^[0-9]+:|inet " || true
    echo ""
    echo "=== 路由表 ==="
    ip route show || true
    echo ""
    echo "=== IP 转发状态 ==="
    echo "IPv4 转发: $(cat /proc/sys/net/ipv4/ip_forward)"
    echo ""
}

# 主函数
main() {
    echo "=== 网络配置安全清理脚本 ==="
    echo "此脚本将清理所有测试相关的网络配置"
    echo "不会影响您原有的网络设置"
    echo ""
    
    check_root
    
    # 询问用户确认
    read -p "确认要清理测试网络配置吗？(y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        log "用户取消操作"
        exit 0
    fi
    
    backup_current_state
    stop_test_processes
    cleanup_tun_devices
    cleanup_test_routes
    cleanup_test_iptables
    restore_ip_forwarding
    cleanup_temp_files
    verify_cleanup
    
    echo ""
    success "=== 网络清理完成 ==="
    echo ""
    show_current_status
    
    if [ -f "/tmp/last_network_backup" ]; then
        echo "网络状态备份位置: $(cat /tmp/last_network_backup)"
    fi
    
    echo ""
    echo "您的原始网络配置已经安全恢复！"
}

# 运行主函数
main "$@"
