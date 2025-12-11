#!/bin/bash

echo "🧪 运行 IPC 学习项目测试..."

# 检查编译文件
programs=("pipe_demo" "msgqueue_demo" "sharedmem_demo" "semaphore_demo" "socket_demo")

for program in "${programs[@]}"; do
    if [ ! -f "./$program" ]; then
        echo "❌ $program 不存在，请先编译"
        echo "运行: ./compile_all.sh"
        exit 1
    fi
done

echo "✅ 所有程序文件存在"

# 运行基本测试
echo ""
echo "🔍 运行基本功能测试..."

echo "1. 测试管道通信..."
timeout 10 ./pipe_demo < /dev/null > /dev/null 2>&1
if [ $? -eq 0 ] || [ $? -eq 124 ]; then
    echo "   ✅ 管道测试完成"
else
    echo "   ❌ 管道测试失败"
fi

echo "2. 测试消息队列..."
timeout 10 ./msgqueue_demo < /dev/null > /dev/null 2>&1
if [ $? -eq 0 ] || [ $? -eq 124 ]; then
    echo "   ✅ 消息队列测试完成"
else
    echo "   ❌ 消息队列测试失败"
fi

echo "3. 测试共享内存..."
timeout 10 ./sharedmem_demo < /dev/null > /dev/null 2>&1
if [ $? -eq 0 ] || [ $? -eq 124 ]; then
    echo "   ✅ 共享内存测试完成"
else
    echo "   ❌ 共享内存测试失败"
fi

echo "4. 测试信号量..."
timeout 10 ./semaphore_demo < /dev/null > /dev/null 2>&1
if [ $? -eq 0 ] || [ $? -eq 124 ]; then
    echo "   ✅ 信号量测试完成"
else
    echo "   ❌ 信号量测试失败"
fi

echo "5. 测试套接字通信..."
timeout 10 ./socket_demo < /dev/null > /dev/null 2>&1
if [ $? -eq 0 ] || [ $? -eq 124 ]; then
    echo "   ✅ 套接字测试完成"
else
    echo "   ❌ 套接字测试失败"
fi

echo ""
echo "🎉 所有测试完成！"
echo ""
echo "📚 学习建议："
echo "  1. 按顺序运行每个示例程序"
echo "  2. 仔细阅读程序输出和说明"
echo "  3. 查看源代码了解实现细节"
echo "  4. 尝试修改代码进行实验"
echo ""
echo "🚀 开始学习："
echo "  ./pipe_demo      # 从管道通信开始"
