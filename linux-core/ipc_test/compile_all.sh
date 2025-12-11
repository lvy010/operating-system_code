#!/bin/bash

echo "🔨 编译 Linux IPC 学习项目..."

# 编译管道示例
echo "编译管道通信示例..."
gcc -Wall -Wextra -std=c99 -o pipe_demo 1_pipe_demo.c
if [ $? -eq 0 ]; then
    echo "✅ pipe_demo 编译成功"
else
    echo "❌ pipe_demo 编译失败"
fi

# 编译消息队列示例
echo "编译消息队列示例..."
gcc -Wall -Wextra -std=c99 -o msgqueue_demo 2_msgqueue_demo.c
if [ $? -eq 0 ]; then
    echo "✅ msgqueue_demo 编译成功"
else
    echo "❌ msgqueue_demo 编译失败"
fi

# 编译共享内存示例
echo "编译共享内存示例..."
gcc -Wall -Wextra -std=c99 -o sharedmem_demo 3_sharedmem_demo.c
if [ $? -eq 0 ]; then
    echo "✅ sharedmem_demo 编译成功"
else
    echo "❌ sharedmem_demo 编译失败"
fi

# 编译信号量示例
echo "编译信号量示例..."
gcc -Wall -Wextra -std=c99 -o semaphore_demo 4_semaphore_demo.c
if [ $? -eq 0 ]; then
    echo "✅ semaphore_demo 编译成功"
else
    echo "❌ semaphore_demo 编译失败"
fi

# 编译套接字示例
echo "编译套接字通信示例..."
gcc -Wall -Wextra -std=c99 -o socket_demo 5_socket_demo.c -lpthread
if [ $? -eq 0 ]; then
    echo "✅ socket_demo 编译成功"
else
    echo "❌ socket_demo 编译失败"
fi

echo ""
echo "🎉 编译完成！"
echo "运行示例："
echo "  ./pipe_demo      # 管道通信"
echo "  ./msgqueue_demo  # 消息队列"
echo "  ./sharedmem_demo # 共享内存"
echo "  ./semaphore_demo # 信号量"
echo "  ./socket_demo    # 套接字通信"
