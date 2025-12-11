#!/bin/bash

# VPN 服务器设置脚本
# 使用 TUN 设备创建简单的 VPN 服务器

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TUN_DEVICE="tun0"
TUN_IP="10.0.0.1"
TUN_NETMASK="24"
INTERNAL_NETWORK="10.0.0.0/24"
EXTERNAL_INTERFACE=""
LOG_FILE="/var/log/tun_vpn.log"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 日志函数
log() {
    echo -e "${GREEN}[$(date '+%Y-%m-%d %H:%M:%S')]${NC} $1" | tee -a "$LOG_FILE"
}

error() {
    echo -e "${RED}[错误]${NC} $1" | tee -a "$LOG_FILE"
}

warning() {
    echo -e "${YELLOW}[警告]${NC} $1" | tee -a "$LOG_FILE"
}

# 检查权限
check_root() {
    if [ "$EUID" -ne 0 ]; then
        error "此脚本需要 root 权限运行"
        echo "请使用: sudo $0"
        exit 1
    fi
}

# 检查依赖
check_dependencies() {
    log "检查系统依赖..."
    
    local missing_deps=()
    
    # 检查必要的命令
    for cmd in ip iptables modprobe; do
        if ! command -v "$cmd" > /dev/null 2>&1; then
            missing_deps+=("$cmd")
        fi
    done
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        error "缺少必要的依赖: ${missing_deps[*]}"
        echo "请安装: apt-get install iproute2 iptables kmod"
        exit 1
    fi
    
    # 检查 TUN 模块
    if ! lsmod | grep -q tun; then
        log "加载 TUN 内核模块..."
        modprobe tun || {
            error "无法加载 TUN 内核模块"
            exit 1
        }
    fi
    
    # 检查 TUN 设备文件
    if [ ! -c "/dev/net/tun" ]; then
        error "/dev/net/tun 设备文件不存在"
        exit 1
    fi
    
    log "依赖检查完成"
}

# 自动检测外部网络接口
detect_external_interface() {
    log "自动检测外部网络接口..."
    
    # 获取默认路由的接口
    EXTERNAL_INTERFACE=$(ip route | grep '^default' | awk '{print $5}' | head -1)
    
    if [ -z "$EXTERNAL_INTERFACE" ]; then
        warning "无法自动检测外部接口，请手动指定"
        echo "可用的网络接口:"
        ip link show | grep -E '^[0-9]+:' | awk -F': ' '{print "  " $2}'
        read -p "请输入外部接口名称: " EXTERNAL_INTERFACE
        
        if [ -z "$EXTERNAL_INTERFACE" ]; then
            error "必须指定外部接口"
            exit 1
        fi
    fi
    
    log "使用外部接口: $EXTERNAL_INTERFACE"
}

# 编译 TUN 路由器程序
build_tun_router() {
    log "编译 TUN 路由器程序..."
    
    cd "$SCRIPT_DIR"
    
    if [ ! -f "tun_router.c" ] || [ ! -f "network_utils.c" ]; then
        error "源代码文件不存在"
        exit 1
    fi
    
    gcc -Wall -Wextra -std=c99 -O2 -o tun_router tun_router.c network_utils.c || {
        error "编译失败"
        exit 1
    }
    
    log "编译完成"
}

# 创建配置文件
create_config() {
    log "创建配置文件..."
    
    cat > "$SCRIPT_DIR/vpn_server.conf" << EOF
# TUN VPN 服务器配置文件
TUN_DEVICE=$TUN_DEVICE
TUN_IP=$TUN_IP
TUN_NETMASK=$TUN_NETMASK
INTERNAL_NETWORK=$INTERNAL_NETWORK
EXTERNAL_INTERFACE=$EXTERNAL_INTERFACE
ENABLE_ROUTING=1
ENABLE_NAT=1
ENABLE_FORWARDING=1
LOG_FILE=$LOG_FILE
EOF
    
    log "配置文件已创建: $SCRIPT_DIR/vpn_server.conf"
}

# 创建启动脚本
create_startup_script() {
    log "创建启动脚本..."
    
    cat > "$SCRIPT_DIR/start_vpn_server.sh" << 'EOF'
#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_FILE="$SCRIPT_DIR/vpn_server.conf"

# 加载配置
if [ -f "$CONFIG_FILE" ]; then
    source "$CONFIG_FILE"
else
    echo "配置文件不存在: $CONFIG_FILE"
    exit 1
fi

# 检查权限
if [ "$EUID" -ne 0 ]; then
    echo "需要 root 权限运行"
    echo "请使用: sudo $0"
    exit 1
fi

echo "启动 TUN VPN 服务器..."
echo "配置:"
echo "  TUN 设备: $TUN_DEVICE"
echo "  TUN IP: $TUN_IP/$TUN_NETMASK"
echo "  内部网络: $INTERNAL_NETWORK"
echo "  外部接口: $EXTERNAL_INTERFACE"
echo "  日志文件: $LOG_FILE"

# 启动 TUN 路由器
cd "$SCRIPT_DIR"
exec ./tun_router \
    -d "$TUN_DEVICE" \
    -i "$TUN_IP" \
    -m "$TUN_NETMASK" \
    -e "$EXTERNAL_INTERFACE" \
    -n "$INTERNAL_NETWORK" \
    -r -N -f
EOF
    
    chmod +x "$SCRIPT_DIR/start_vpn_server.sh"
    log "启动脚本已创建: $SCRIPT_DIR/start_vpn_server.sh"
}

# 创建停止脚本
create_stop_script() {
    log "创建停止脚本..."
    
    cat > "$SCRIPT_DIR/stop_vpn_server.sh" << 'EOF'
#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_FILE="$SCRIPT_DIR/vpn_server.conf"

# 加载配置
if [ -f "$CONFIG_FILE" ]; then
    source "$CONFIG_FILE"
else
    echo "配置文件不存在: $CONFIG_FILE"
    exit 1
fi

# 检查权限
if [ "$EUID" -ne 0 ]; then
    echo "需要 root 权限运行"
    echo "请使用: sudo $0"
    exit 1
fi

echo "停止 TUN VPN 服务器..."

# 终止 tun_router 进程
pkill -f "tun_router" || true

# 清理网络配置
echo "清理网络配置..."

# 删除路由
ip route del "$INTERNAL_NETWORK" dev "$TUN_DEVICE" 2>/dev/null || true

# 清理 iptables 规则
iptables -t nat -D POSTROUTING -o "$EXTERNAL_INTERFACE" -j MASQUERADE 2>/dev/null || true
iptables -t filter -D FORWARD -i "$TUN_DEVICE" -o "$EXTERNAL_INTERFACE" -j ACCEPT 2>/dev/null || true
iptables -t filter -D FORWARD -i "$EXTERNAL_INTERFACE" -o "$TUN_DEVICE" -j ACCEPT 2>/dev/null || true

# 关闭 TUN 接口
ip link set "$TUN_DEVICE" down 2>/dev/null || true

echo "VPN 服务器已停止"
EOF
    
    chmod +x "$SCRIPT_DIR/stop_vpn_server.sh"
    log "停止脚本已创建: $SCRIPT_DIR/stop_vpn_server.sh"
}

# 创建状态检查脚本
create_status_script() {
    log "创建状态检查脚本..."
    
    cat > "$SCRIPT_DIR/vpn_status.sh" << 'EOF'
#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_FILE="$SCRIPT_DIR/vpn_server.conf"

# 加载配置
if [ -f "$CONFIG_FILE" ]; then
    source "$CONFIG_FILE"
else
    echo "配置文件不存在: $CONFIG_FILE"
    exit 1
fi

echo "=== TUN VPN 服务器状态 ==="

# 检查进程
if pgrep -f "tun_router" > /dev/null; then
    echo "✓ TUN 路由器进程运行中"
    echo "  PID: $(pgrep -f "tun_router")"
else
    echo "✗ TUN 路由器进程未运行"
fi

# 检查 TUN 接口
if ip link show "$TUN_DEVICE" > /dev/null 2>&1; then
    echo "✓ TUN 接口存在: $TUN_DEVICE"
    ip addr show "$TUN_DEVICE" | grep inet
else
    echo "✗ TUN 接口不存在: $TUN_DEVICE"
fi

# 检查路由
if ip route show | grep -q "$INTERNAL_NETWORK"; then
    echo "✓ 内部网络路由已配置"
    ip route show | grep "$INTERNAL_NETWORK"
else
    echo "✗ 内部网络路由未配置"
fi

# 检查 IP 转发
if [ "$(cat /proc/sys/net/ipv4/ip_forward)" = "1" ]; then
    echo "✓ IP 转发已启用"
else
    echo "✗ IP 转发未启用"
fi

# 检查 NAT 规则
if iptables -t nat -L POSTROUTING | grep -q "MASQUERADE"; then
    echo "✓ NAT 规则已配置"
else
    echo "✗ NAT 规则未配置"
fi

# 显示连接统计
echo ""
echo "=== 连接统计 ==="
if [ -f "$LOG_FILE" ]; then
    echo "日志文件: $LOG_FILE"
    echo "最近的日志条目:"
    tail -5 "$LOG_FILE" 2>/dev/null || echo "无日志内容"
else
    echo "日志文件不存在"
fi

echo ""
echo "=== 网络接口信息 ==="
ip addr show "$TUN_DEVICE" 2>/dev/null || echo "TUN 接口未找到"

echo ""
echo "========================="
EOF
    
    chmod +x "$SCRIPT_DIR/vpn_status.sh"
    log "状态检查脚本已创建: $SCRIPT_DIR/vpn_status.sh"
}

# 创建客户端连接脚本
create_client_script() {
    log "创建客户端连接脚本..."
    
    cat > "$SCRIPT_DIR/client_connect.sh" << 'EOF'
#!/bin/bash

# TUN VPN 客户端连接脚本
# 此脚本演示如何配置客户端连接到 VPN 服务器

SERVER_IP="<VPN_SERVER_IP>"
TUN_DEVICE="tun1"
CLIENT_IP="10.0.0.100"
SERVER_TUN_IP="10.0.0.1"

echo "=== TUN VPN 客户端配置 ==="
echo "注意: 这是一个示例脚本，实际使用时需要根据具体情况修改"
echo ""

# 检查权限
if [ "$EUID" -ne 0 ]; then
    echo "需要 root 权限运行"
    echo "请使用: sudo $0"
    exit 1
fi

echo "配置步骤:"
echo "1. 创建 TUN 设备"
echo "2. 配置 IP 地址"
echo "3. 添加路由"
echo "4. 测试连接"

echo ""
echo "手动配置命令:"
echo "# 创建 TUN 设备 (需要专门的程序)"
echo "# ./tun_test -d $TUN_DEVICE -t"
echo ""
echo "# 配置客户端 IP"
echo "ip addr add $CLIENT_IP/24 dev $TUN_DEVICE"
echo "ip link set $TUN_DEVICE up"
echo ""
echo "# 添加到服务器的路由"
echo "ip route add 10.0.0.0/24 dev $TUN_DEVICE"
echo ""
echo "# 测试连接"
echo "ping $SERVER_TUN_IP"

echo ""
echo "注意: 实际的 VPN 连接需要更复杂的握手和加密机制"
echo "这个脚本仅用于演示基本的网络配置"
EOF
    
    chmod +x "$SCRIPT_DIR/client_connect.sh"
    log "客户端连接脚本已创建: $SCRIPT_DIR/client_connect.sh"
}

# 显示使用说明
show_usage() {
    echo ""
    log "=== VPN 服务器设置完成 ==="
    echo ""
    echo "创建的文件:"
    echo "  vpn_server.conf      - 配置文件"
    echo "  start_vpn_server.sh  - 启动脚本"
    echo "  stop_vpn_server.sh   - 停止脚本"
    echo "  vpn_status.sh        - 状态检查脚本"
    echo "  client_connect.sh    - 客户端连接示例"
    echo ""
    echo "使用方法:"
    echo "  启动服务: sudo ./start_vpn_server.sh"
    echo "  停止服务: sudo ./stop_vpn_server.sh"
    echo "  检查状态: ./vpn_status.sh"
    echo ""
    echo "服务器配置:"
    echo "  TUN 设备: $TUN_DEVICE"
    echo "  服务器 IP: $TUN_IP/$TUN_NETMASK"
    echo "  内部网络: $INTERNAL_NETWORK"
    echo "  外部接口: $EXTERNAL_INTERFACE"
    echo ""
    echo "客户端连接:"
    echo "  1. 客户端需要创建 TUN 设备"
    echo "  2. 配置客户端 IP (如: 10.0.0.100/24)"
    echo "  3. 添加路由到服务器网络"
    echo "  4. 建立数据传输通道"
    echo ""
    echo "日志文件: $LOG_FILE"
}

# 主函数
main() {
    log "开始设置 TUN VPN 服务器..."
    
    check_root
    check_dependencies
    detect_external_interface
    build_tun_router
    create_config
    create_startup_script
    create_stop_script
    create_status_script
    create_client_script
    show_usage
    
    log "VPN 服务器设置完成！"
}

# 运行主函数
main "$@"
