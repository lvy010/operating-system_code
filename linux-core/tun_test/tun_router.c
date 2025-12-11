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
#include <signal.h>
#include "network_utils.h"

// 全局变量
static int tun_fd = -1;
static char tun_device[IFNAMSIZ] = "tun0";
static int running = 1;

// 配置选项结构体
typedef struct {
    int enable_routing;         // 启用路由功能
    int enable_nat;            // 启用 NAT
    int enable_forwarding;     // 启用 IP 转发
    char tun_ip[32];           // TUN 接口 IP
    char tun_netmask[32];      // TUN 接口子网掩码
    char external_interface[32]; // 外部接口名称
    char internal_network[32];   // 内部网络段
    int port_forward_enabled;    // 端口转发开关
    int external_port;          // 外部端口
    char internal_ip[32];       // 内部 IP
    int internal_port;          // 内部端口
    char forward_protocol[16];  // 转发协议
} router_config_t;

// 信号处理函数
void signal_handler(int sig) {
    printf("\n收到信号 %d，正在清理资源...\n", sig);
    running = 0;
    if (tun_fd >= 0) {
        close(tun_fd);
    }
}

/**
 * TUN 设备分配函数
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
 * 配置 TUN 路由器
 */
int configure_tun_router(const router_config_t *config) {
    network_config_t net_config = {0};
    route_config_t route_config = {0};
    iptables_rule_t iptables_rule = {0};
    
    printf("\n=== 配置 TUN 路由器 ===\n");
    
    // 配置 TUN 接口
    strncpy(net_config.interface, tun_device, sizeof(net_config.interface) - 1);
    strncpy(net_config.ip_addr, config->tun_ip, sizeof(net_config.ip_addr) - 1);
    strncpy(net_config.netmask, config->tun_netmask, sizeof(net_config.netmask) - 1);
    net_config.mtu = 1500;
    
    if (configure_interface(&net_config) != 0) {
        fprintf(stderr, "TUN 接口配置失败\n");
        return -1;
    }
    
    // 启用 IP 转发
    if (config->enable_forwarding) {
        printf("启用 IP 转发...\n");
        execute_command("echo 1 > /proc/sys/net/ipv4/ip_forward");
    }
    
    // 配置路由
    if (config->enable_routing && strlen(config->internal_network) > 0) {
        printf("配置内部网络路由...\n");
        strncpy(route_config.destination, config->internal_network, sizeof(route_config.destination) - 1);
        strncpy(route_config.interface, tun_device, sizeof(route_config.interface) - 1);
        route_config.metric = 100;
        add_route(&route_config);
    }
    
    // 配置 NAT
    if (config->enable_nat && strlen(config->external_interface) > 0) {
        printf("配置 NAT 伪装...\n");
        setup_nat_masquerade(config->external_interface);
        
        // 允许转发流量
        strncpy(iptables_rule.table, "filter", sizeof(iptables_rule.table) - 1);
        strncpy(iptables_rule.chain, "FORWARD", sizeof(iptables_rule.chain) - 1);
        strncpy(iptables_rule.action, "ACCEPT", sizeof(iptables_rule.action) - 1);
        strncpy(iptables_rule.interface_in, tun_device, sizeof(iptables_rule.interface_in) - 1);
        strncpy(iptables_rule.interface_out, config->external_interface, sizeof(iptables_rule.interface_out) - 1);
        add_iptables_rule(&iptables_rule);
        
        // 允许返回流量
        memset(&iptables_rule, 0, sizeof(iptables_rule));
        strncpy(iptables_rule.table, "filter", sizeof(iptables_rule.table) - 1);
        strncpy(iptables_rule.chain, "FORWARD", sizeof(iptables_rule.chain) - 1);
        strncpy(iptables_rule.action, "ACCEPT", sizeof(iptables_rule.action) - 1);
        strncpy(iptables_rule.interface_in, config->external_interface, sizeof(iptables_rule.interface_in) - 1);
        strncpy(iptables_rule.interface_out, tun_device, sizeof(iptables_rule.interface_out) - 1);
        add_iptables_rule(&iptables_rule);
    }
    
    // 配置端口转发
    if (config->port_forward_enabled && config->external_port > 0) {
        printf("配置端口转发: %d -> %s:%d (%s)\n", 
               config->external_port, config->internal_ip, 
               config->internal_port, config->forward_protocol);
        
        setup_port_forwarding(config->external_port, config->internal_ip, 
                            config->internal_port, config->forward_protocol);
    }
    
    printf("TUN 路由器配置完成\n");
    return 0;
}

/**
 * 清理网络配置
 */
void cleanup_network_config(const router_config_t *config) {
    printf("\n=== 清理网络配置 ===\n");
    
    // 删除路由
    if (config->enable_routing && strlen(config->internal_network) > 0) {
        route_config_t route_config = {0};
        strncpy(route_config.destination, config->internal_network, sizeof(route_config.destination) - 1);
        strncpy(route_config.interface, tun_device, sizeof(route_config.interface) - 1);
        delete_route(&route_config);
    }
    
    // 清理 iptables 规则
    if (config->enable_nat) {
        printf("清理 iptables 规则...\n");
        execute_command("iptables -t nat -F POSTROUTING");
        execute_command("iptables -t nat -F PREROUTING");
        execute_command("iptables -t filter -F FORWARD");
    }
    
    // 关闭 TUN 接口
    if (strlen(tun_device) > 0) {
        bring_interface_down(tun_device);
    }
    
    printf("网络配置清理完成\n");
}

/**
 * 数据包转发处理
 */
void handle_packet_forwarding(int tun_fd) {
    unsigned char buf[2048];
    int len;
    
    printf("开始数据包转发服务...\n");
    printf("按 Ctrl+C 停止服务\n\n");
    
    while (running) {
        len = read(tun_fd, buf, sizeof(buf));
        if (len < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(1000); // 1ms
                continue;
            }
            if (running) {
                perror("读取 TUN 设备失败");
            }
            break;
        }
        
        if (len > 0) {
            // 简单的数据包统计
            static int packet_count = 0;
            packet_count++;
            
            if (packet_count % 100 == 0) {
                printf("已转发 %d 个数据包\n", packet_count);
            }
            
            // 这里可以添加更复杂的数据包处理逻辑
            // 例如：数据包过滤、修改、记录等
        }
    }
}

/**
 * 显示帮助信息
 */
void show_help(const char *prog_name) {
    printf("用法: %s [选项]\n", prog_name);
    printf("TUN 设备路由器 - 集成路由和防火墙功能\n\n");
    printf("选项:\n");
    printf("  -d <设备名>      指定 TUN 设备名称 (默认: tun0)\n");
    printf("  -i <IP地址>      TUN 接口 IP 地址 (默认: 10.0.0.1)\n");
    printf("  -m <子网掩码>    TUN 接口子网掩码 (默认: 24)\n");
    printf("  -e <外部接口>    外部网络接口名称 (如: eth0)\n");
    printf("  -n <内部网络>    内部网络段 (如: 10.0.0.0/24)\n");
    printf("  -r               启用路由功能\n");
    printf("  -N               启用 NAT 功能\n");
    printf("  -f               启用 IP 转发\n");
    printf("  -p <外部端口>    端口转发外部端口\n");
    printf("  -t <内部IP>      端口转发目标 IP\n");
    printf("  -P <内部端口>    端口转发目标端口\n");
    printf("  -T <协议>        端口转发协议 (tcp/udp)\n");
    printf("  -c               仅配置不运行转发服务\n");
    printf("  -s               显示当前网络状态\n");
    printf("  -h               显示此帮助信息\n");
    printf("\n");
    printf("示例:\n");
    printf("  # 基本 TUN 路由器\n");
    printf("  %s -r -f -e eth0 -n 10.0.0.0/24\n", prog_name);
    printf("\n");
    printf("  # 带 NAT 的路由器\n");
    printf("  %s -r -N -f -e eth0 -n 10.0.0.0/24\n", prog_name);
    printf("\n");
    printf("  # 带端口转发的路由器\n");
    printf("  %s -r -N -f -e eth0 -p 8080 -t 10.0.0.100 -P 80 -T tcp\n", prog_name);
}

/**
 * 显示网络状态
 */
void show_network_status(void) {
    printf("\n=== 网络状态信息 ===\n");
    
    printf("\n--- 网络接口 ---\n");
    execute_command("ip addr show");
    
    printf("\n--- 路由表 ---\n");
    show_routes();
    
    printf("\n--- iptables 规则 ---\n");
    printf("NAT 表:\n");
    execute_command("iptables -t nat -L -n");
    printf("\nFilter 表:\n");
    execute_command("iptables -t filter -L -n");
    
    printf("\n--- IP 转发状态 ---\n");
    execute_command("cat /proc/sys/net/ipv4/ip_forward");
    
    printf("\n===================\n");
}

int main(int argc, char *argv[]) {
    router_config_t config = {0};
    int opt;
    int config_only = 0;
    int show_status = 0;
    
    // 默认配置
    strncpy(config.tun_ip, "10.0.0.1", sizeof(config.tun_ip) - 1);
    strncpy(config.tun_netmask, "24", sizeof(config.tun_netmask) - 1);
    strncpy(config.forward_protocol, "tcp", sizeof(config.forward_protocol) - 1);
    
    printf("=== TUN 设备路由器 ===\n");
    
    // 解析命令行参数
    while ((opt = getopt(argc, argv, "d:i:m:e:n:rNfp:t:P:T:csh")) != -1) {
        switch (opt) {
            case 'd':
                strncpy(tun_device, optarg, IFNAMSIZ - 1);
                break;
            case 'i':
                strncpy(config.tun_ip, optarg, sizeof(config.tun_ip) - 1);
                break;
            case 'm':
                strncpy(config.tun_netmask, optarg, sizeof(config.tun_netmask) - 1);
                break;
            case 'e':
                strncpy(config.external_interface, optarg, sizeof(config.external_interface) - 1);
                break;
            case 'n':
                strncpy(config.internal_network, optarg, sizeof(config.internal_network) - 1);
                break;
            case 'r':
                config.enable_routing = 1;
                break;
            case 'N':
                config.enable_nat = 1;
                break;
            case 'f':
                config.enable_forwarding = 1;
                break;
            case 'p':
                config.external_port = atoi(optarg);
                config.port_forward_enabled = 1;
                break;
            case 't':
                strncpy(config.internal_ip, optarg, sizeof(config.internal_ip) - 1);
                break;
            case 'P':
                config.internal_port = atoi(optarg);
                break;
            case 'T':
                strncpy(config.forward_protocol, optarg, sizeof(config.forward_protocol) - 1);
                break;
            case 'c':
                config_only = 1;
                break;
            case 's':
                show_status = 1;
                break;
            case 'h':
                show_help(argv[0]);
                return 0;
            default:
                show_help(argv[0]);
                return 1;
        }
    }
    
    // 显示状态信息
    if (show_status) {
        show_network_status();
        return 0;
    }
    
    // 检查权限
    if (geteuid() != 0) {
        fprintf(stderr, "错误: 此程序需要 root 权限运行\n");
        fprintf(stderr, "请使用: sudo %s\n", argv[0]);
        return 1;
    }
    
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // 创建 TUN 设备
    printf("创建 TUN 设备: %s\n", tun_device);
    tun_fd = tun_alloc(tun_device);
    if (tun_fd < 0) {
        fprintf(stderr, "TUN 设备创建失败\n");
        return 1;
    }
    
    // 配置路由器
    if (configure_tun_router(&config) != 0) {
        fprintf(stderr, "路由器配置失败\n");
        cleanup_network_config(&config);
        close(tun_fd);
        return 1;
    }
    
    // 显示配置信息
    printf("\n=== 路由器配置信息 ===\n");
    printf("TUN 设备: %s\n", tun_device);
    printf("TUN IP: %s/%s\n", config.tun_ip, config.tun_netmask);
    printf("路由功能: %s\n", config.enable_routing ? "启用" : "禁用");
    printf("NAT 功能: %s\n", config.enable_nat ? "启用" : "禁用");
    printf("IP 转发: %s\n", config.enable_forwarding ? "启用" : "禁用");
    if (strlen(config.external_interface) > 0) {
        printf("外部接口: %s\n", config.external_interface);
    }
    if (strlen(config.internal_network) > 0) {
        printf("内部网络: %s\n", config.internal_network);
    }
    if (config.port_forward_enabled) {
        printf("端口转发: %d -> %s:%d (%s)\n", 
               config.external_port, config.internal_ip, 
               config.internal_port, config.forward_protocol);
    }
    printf("====================\n");
    
    if (config_only) {
        printf("仅配置模式，程序退出\n");
        close(tun_fd);
        return 0;
    }
    
    // 设置非阻塞模式
    int flags = fcntl(tun_fd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(tun_fd, F_SETFL, flags | O_NONBLOCK);
    }
    
    // 开始数据包转发
    handle_packet_forwarding(tun_fd);
    
    // 清理资源
    cleanup_network_config(&config);
    close(tun_fd);
    
    printf("TUN 路由器已停止\n");
    return 0;
}
