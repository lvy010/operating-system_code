#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/**
 * TUN 设备分配函数
 * @param dev 设备名称
 * @return 成功返回文件描述符，失败返回负数
 */
int tun_alloc(char *dev) {
    struct ifreq ifr;
    int fd, err;

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        perror("Opening /dev/net/tun");
        return fd;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
        close(fd);
        perror("ioctl(TUNSETIFF)");
        return err;
    }

    printf("TUN 设备创建成功: %s\n", dev);
    return fd;
}

/**
 * 打印数据包内容（十六进制格式）
 */
void print_packet(unsigned char *buf, int len) {
    printf("收到数据包 (%d 字节): ", len);
    for (int i = 0; i < len && i < 32; i++) {  // 只显示前32字节
        printf("%02x ", buf[i]);
    }
    if (len > 32) printf("...");
    printf("\n");
}

/**
 * 简单的数据包分析
 */
void analyze_packet(unsigned char *buf, int len) {
    if (len < 20) {
        printf("数据包太短，无法分析\n");
        return;
    }
    
    // 假设是 IPv4 数据包
    unsigned char version = (buf[0] >> 4) & 0x0F;
    unsigned char protocol = buf[9];
    
    printf("IP版本: %d\n", version);
    if (version == 4) {
        struct in_addr src_addr, dst_addr;
        memcpy(&src_addr, &buf[12], 4);
        memcpy(&dst_addr, &buf[16], 4);
        
        printf("源地址: %s\n", inet_ntoa(src_addr));
        printf("目标地址: %s\n", inet_ntoa(dst_addr));
        printf("协议: %d ", protocol);
        
        switch (protocol) {
            case 1: printf("(ICMP)"); break;
            case 6: printf("(TCP)"); break;
            case 17: printf("(UDP)"); break;
            default: printf("(其他)"); break;
        }
        printf("\n");
    }
}

/**
 * 测试 TUN 设备读取功能
 */
void test_tun_read(int tun_fd) {
    unsigned char buf[2048];
    int len;
    
    printf("\n开始监听 TUN 设备数据包...\n");
    printf("提示: 您可以在另一个终端中使用以下命令测试:\n");
    printf("  sudo ip addr add 10.0.0.1/24 dev tun0\n");
    printf("  sudo ip link set tun0 up\n");
    printf("  ping 10.0.0.2\n");
    printf("按 Ctrl+C 退出监听\n\n");
    
    while (1) {
        len = read(tun_fd, buf, sizeof(buf));
        if (len < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            perror("读取 TUN 设备失败");
            break;
        }
        
        printf("\n=== 新数据包 ===\n");
        print_packet(buf, len);
        analyze_packet(buf, len);
        printf("================\n");
    }
}

/**
 * 显示帮助信息
 */
void show_help(const char *prog_name) {
    printf("用法: %s [选项]\n", prog_name);
    printf("选项:\n");
    printf("  -d <设备名>  指定 TUN 设备名称 (默认: tun0)\n");
    printf("  -t           仅测试设备创建，不进行数据读取\n");
    printf("  -h           显示此帮助信息\n");
    printf("\n");
    printf("示例:\n");
    printf("  %s                    # 创建 tun0 设备并监听数据\n", prog_name);
    printf("  %s -d mytun          # 创建 mytun 设备并监听数据\n", prog_name);
    printf("  %s -t                # 仅测试设备创建\n", prog_name);
}

int main(int argc, char *argv[]) {
    char dev_name[IFNAMSIZ] = "tun0";
    int tun_fd;
    int test_only = 0;
    int opt;
    
    printf("=== TUN 设备测试程序 ===\n");
    
    // 解析命令行参数
    while ((opt = getopt(argc, argv, "d:th")) != -1) {
        switch (opt) {
            case 'd':
                strncpy(dev_name, optarg, IFNAMSIZ - 1);
                dev_name[IFNAMSIZ - 1] = '\0';
                break;
            case 't':
                test_only = 1;
                break;
            case 'h':
                show_help(argv[0]);
                return 0;
            default:
                show_help(argv[0]);
                return 1;
        }
    }
    
    // 检查是否以 root 权限运行
    if (geteuid() != 0) {
        fprintf(stderr, "错误: 此程序需要 root 权限运行\n");
        fprintf(stderr, "请使用: sudo %s\n", argv[0]);
        return 1;
    }
    
    printf("尝试创建 TUN 设备: %s\n", dev_name);
    
    // 创建 TUN 设备
    tun_fd = tun_alloc(dev_name);
    if (tun_fd < 0) {
        fprintf(stderr, "TUN 设备创建失败\n");
        return 1;
    }
    
    printf("TUN 设备文件描述符: %d\n", tun_fd);
    
    if (test_only) {
        printf("测试模式: 设备创建成功，程序退出\n");
        close(tun_fd);
        return 0;
    }
    
    // 设置非阻塞模式（可选）
    int flags = fcntl(tun_fd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(tun_fd, F_SETFL, flags | O_NONBLOCK);
    }
    
    // 开始读取数据
    test_tun_read(tun_fd);
    
    // 清理
    close(tun_fd);
    printf("TUN 设备已关闭\n");
    
    return 0;
}
