#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// 网络配置结构体
typedef struct {
    char interface[32];     // 接口名称
    char ip_addr[32];       // IP 地址
    char netmask[32];       // 子网掩码
    char gateway[32];       // 网关地址
    int mtu;                // MTU 大小
} network_config_t;

// 路由配置结构体
typedef struct {
    char destination[32];   // 目标网络
    char gateway[32];       // 网关
    char interface[32];     // 出接口
    int metric;             // 路由优先级
} route_config_t;

// iptables 规则结构体
typedef struct {
    char table[16];         // 表名 (filter, nat, mangle)
    char chain[16];         // 链名 (INPUT, OUTPUT, FORWARD, etc.)
    char action[16];        // 动作 (ACCEPT, DROP, REJECT, MASQUERADE)
    char protocol[16];      // 协议 (tcp, udp, icmp, all)
    char source[32];        // 源地址
    char destination[32];   // 目标地址
    char interface_in[32];  // 入接口
    char interface_out[32]; // 出接口
    int port;               // 端口号
} iptables_rule_t;

// 函数声明

// 网络接口配置
int configure_interface(const network_config_t *config);
int bring_interface_up(const char *interface);
int bring_interface_down(const char *interface);
int set_interface_ip(const char *interface, const char *ip, const char *netmask);
int set_interface_mtu(const char *interface, int mtu);

// 路由配置
int add_route(const route_config_t *route);
int delete_route(const route_config_t *route);
int add_default_route(const char *gateway, const char *interface);
int delete_default_route(void);
int show_routes(void);

// iptables 配置
int add_iptables_rule(const iptables_rule_t *rule);
int delete_iptables_rule(const iptables_rule_t *rule);
int flush_iptables_chain(const char *table, const char *chain);
int setup_nat_masquerade(const char *out_interface);
int setup_port_forwarding(int external_port, const char *internal_ip, int internal_port, const char *protocol);

// 网络诊断
int ping_host(const char *host, int count);
int traceroute_host(const char *host);
int check_connectivity(const char *host, int port);

// 工具函数
int execute_command(const char *command);
int execute_command_with_output(const char *command, char *output, size_t output_size);
void print_network_config(const network_config_t *config);
void print_route_config(const route_config_t *route);
void print_iptables_rule(const iptables_rule_t *rule);

#endif // NETWORK_UTILS_H
