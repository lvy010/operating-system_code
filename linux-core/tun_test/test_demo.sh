#!/bin/bash

# TUN 设备测试演示脚本

echo "=== TUN 设备测试演示 ==="
echo ""

# 检查是否以 root 权限运行
if [ "$EUID" -ne 0 ]; then
    echo "错误: 此脚本需要 root 权限运行"
    echo "请使用: sudo $0"
    exit 1
fi

# 检查程序是否存在
if [ ! -f "./tun_test" ]; then
    echo "错误: tun_test 程序不存在，请先编译"
    echo "运行: make"
    exit 1
fi

echo "1. 测试 TUN 设备创建..."
./tun_test -t
if [ $? -eq 0 ]; then
    echo "✓ TUN 设备创建测试成功"
else
    echo "✗ TUN 设备创建测试失败"
    exit 1
fi

echo ""
echo "2. 测试自定义设备名称..."
./tun_test -d test_tun -t
if [ $? -eq 0 ]; then
    echo "✓ 自定义设备名称测试成功"
else
    echo "✗ 自定义设备名称测试失败"
    exit 1
fi

echo ""
echo "3. 检查系统中的 TUN 支持..."

# 检查 tun 模块
if lsmod | grep -q tun; then
    echo "✓ TUN 内核模块已加载"
else
    echo "⚠ TUN 内核模块未加载，尝试加载..."
    modprobe tun
    if [ $? -eq 0 ]; then
        echo "✓ TUN 内核模块加载成功"
    else
        echo "✗ TUN 内核模块加载失败"
    fi
fi

# 检查设备文件
if [ -c "/dev/net/tun" ]; then
    echo "✓ /dev/net/tun 设备文件存在"
    ls -l /dev/net/tun
else
    echo "✗ /dev/net/tun 设备文件不存在"
fi

echo ""
echo "4. 完整功能测试说明："
echo "   要测试完整的数据包监听功能，请执行以下步骤："
echo ""
echo "   终端 1 (启动监听程序):"
echo "   sudo ./tun_test"
echo ""
echo "   终端 2 (配置网络接口):"
echo "   sudo ip addr add 10.0.0.1/24 dev tun0"
echo "   sudo ip link set tun0 up"
echo "   ping 10.0.0.2"
echo ""
echo "   程序将显示接收到的 ICMP 数据包信息"

echo ""
echo "=== 测试完成 ==="
