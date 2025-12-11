#include "network_utils.h"

// 执行系统命令
int execute_command(const char *command) {
    printf("执行命令: %s\n", command);
    int result = system(command);
    if (result == -1) {
        perror("system() 失败");
        return -1;
    }
    return WEXITSTATUS(result);
}

// 执行命令并获取输出
int execute_command_with_output(const char *command, char *output, size_t output_size) {
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen() 失败");
        return -1;
    }
    
    if (fgets(output, output_size, fp) == NULL) {
        output[0] = '\0';
    }
    
    int result = pclose(fp);
    return WEXITSTATUS(result);
}

// 配置网络接口
int configure_interface(const network_config_t *config) {
    char command[256];
    int result = 0;
    
    printf("配置网络接口: %s\n", config->interface);
    
    // 设置 IP 地址和子网掩码
    if (strlen(config->ip_addr) > 0 && strlen(config->netmask) > 0) {
        result |= set_interface_ip(config->interface, config->ip_addr, config->netmask);
    }
    
    // 设置 MTU
    if (config->mtu > 0) {
        result |= set_interface_mtu(config->interface, config->mtu);
    }
    
    // 启用接口
    result |= bring_interface_up(config->interface);
    
    return result;
}

// 启用网络接口
int bring_interface_up(const char *interface) {
    char command[128];
    snprintf(command, sizeof(command), "ip link set %s up", interface);
    return execute_command(command);
}

// 禁用网络接口
int bring_interface_down(const char *interface) {
    char command[128];
    snprintf(command, sizeof(command), "ip link set %s down", interface);
    return execute_command(command);
}

// 设置接口 IP 地址
int set_interface_ip(const char *interface, const char *ip, const char *netmask) {
    char command[256];
    
    // 首先清除现有的 IP 地址
    snprintf(command, sizeof(command), "ip addr flush dev %s", interface);
    execute_command(command);
    
    // 设置新的 IP 地址
    snprintf(command, sizeof(command), "ip addr add %s/%s dev %s", ip, netmask, interface);
    return execute_command(command);
}

// 设置接口 MTU
int set_interface_mtu(const char *interface, int mtu) {
    char command[128];
    snprintf(command, sizeof(command), "ip link set %s mtu %d", interface, mtu);
    return execute_command(command);
}

// 添加路由
int add_route(const route_config_t *route) {
    char command[256];
    
    printf("添加路由: %s -> %s via %s\n", 
           route->destination, route->interface, route->gateway);
    
    if (strlen(route->gateway) > 0) {
        snprintf(command, sizeof(command), 
                "ip route add %s via %s dev %s metric %d",
                route->destination, route->gateway, route->interface, route->metric);
    } else {
        snprintf(command, sizeof(command), 
                "ip route add %s dev %s metric %d",
                route->destination, route->interface, route->metric);
    }
    
    return execute_command(command);
}

// 删除路由
int delete_route(const route_config_t *route) {
    char command[256];
    
    printf("删除路由: %s\n", route->destination);
    
    if (strlen(route->gateway) > 0) {
        snprintf(command, sizeof(command), 
                "ip route del %s via %s dev %s",
                route->destination, route->gateway, route->interface);
    } else {
        snprintf(command, sizeof(command), 
                "ip route del %s dev %s",
                route->destination, route->interface);
    }
    
    return execute_command(command);
}

// 添加默认路由
int add_default_route(const char *gateway, const char *interface) {
    char command[128];
    
    printf("添加默认路由: via %s dev %s\n", gateway, interface);
    
    if (strlen(gateway) > 0) {
        snprintf(command, sizeof(command), "ip route add default via %s dev %s", gateway, interface);
    } else {
        snprintf(command, sizeof(command), "ip route add default dev %s", interface);
    }
    
    return execute_command(command);
}

// 删除默认路由
int delete_default_route(void) {
    printf("删除默认路由\n");
    return execute_command("ip route del default");
}

// 显示路由表
int show_routes(void) {
    printf("\n=== 当前路由表 ===\n");
    return execute_command("ip route show");
}

// 添加 iptables 规则
int add_iptables_rule(const iptables_rule_t *rule) {
    char command[512];
    char rule_str[256] = "";
    
    // 构建规则字符串
    if (strlen(rule->protocol) > 0 && strcmp(rule->protocol, "all") != 0) {
        char proto[64];
        snprintf(proto, sizeof(proto), "-p %s ", rule->protocol);
        strcat(rule_str, proto);
    }
    
    if (strlen(rule->source) > 0) {
        char src[64];
        snprintf(src, sizeof(src), "-s %s ", rule->source);
        strcat(rule_str, src);
    }
    
    if (strlen(rule->destination) > 0) {
        char dst[64];
        snprintf(dst, sizeof(dst), "-d %s ", rule->destination);
        strcat(rule_str, dst);
    }
    
    if (strlen(rule->interface_in) > 0) {
        char iface_in[64];
        snprintf(iface_in, sizeof(iface_in), "-i %s ", rule->interface_in);
        strcat(rule_str, iface_in);
    }
    
    if (strlen(rule->interface_out) > 0) {
        char iface_out[64];
        snprintf(iface_out, sizeof(iface_out), "-o %s ", rule->interface_out);
        strcat(rule_str, iface_out);
    }
    
    if (rule->port > 0) {
        char port_str[32];
        snprintf(port_str, sizeof(port_str), "--dport %d ", rule->port);
        strcat(rule_str, port_str);
    }
    
    snprintf(command, sizeof(command), 
            "iptables -t %s -A %s %s-j %s",
            rule->table, rule->chain, rule_str, rule->action);
    
    printf("添加 iptables 规则: %s\n", command);
    return execute_command(command);
}

// 删除 iptables 规则
int delete_iptables_rule(const iptables_rule_t *rule) {
    char command[512];
    char rule_str[256] = "";
    
    // 构建规则字符串（与添加相同）
    if (strlen(rule->protocol) > 0 && strcmp(rule->protocol, "all") != 0) {
        char proto[64];
        snprintf(proto, sizeof(proto), "-p %s ", rule->protocol);
        strcat(rule_str, proto);
    }
    
    if (strlen(rule->source) > 0) {
        char src[64];
        snprintf(src, sizeof(src), "-s %s ", rule->source);
        strcat(rule_str, src);
    }
    
    if (strlen(rule->destination) > 0) {
        char dst[64];
        snprintf(dst, sizeof(dst), "-d %s ", rule->destination);
        strcat(rule_str, dst);
    }
    
    if (strlen(rule->interface_in) > 0) {
        char iface_in[64];
        snprintf(iface_in, sizeof(iface_in), "-i %s ", rule->interface_in);
        strcat(rule_str, iface_in);
    }
    
    if (strlen(rule->interface_out) > 0) {
        char iface_out[64];
        snprintf(iface_out, sizeof(iface_out), "-o %s ", rule->interface_out);
        strcat(rule_str, iface_out);
    }
    
    if (rule->port > 0) {
        char port_str[32];
        snprintf(port_str, sizeof(port_str), "--dport %d ", rule->port);
        strcat(rule_str, port_str);
    }
    
    snprintf(command, sizeof(command), 
            "iptables -t %s -D %s %s-j %s",
            rule->table, rule->chain, rule_str, rule->action);
    
    printf("删除 iptables 规则: %s\n", command);
    return execute_command(command);
}

// 清空 iptables 链
int flush_iptables_chain(const char *table, const char *chain) {
    char command[128];
    snprintf(command, sizeof(command), "iptables -t %s -F %s", table, chain);
    printf("清空 iptables 链: %s %s\n", table, chain);
    return execute_command(command);
}

// 设置 NAT 伪装
int setup_nat_masquerade(const char *out_interface) {
    char command[256];
    
    printf("设置 NAT 伪装: 出接口 %s\n", out_interface);
    
    // 启用 IP 转发
    execute_command("echo 1 > /proc/sys/net/ipv4/ip_forward");
    
    // 添加 MASQUERADE 规则
    snprintf(command, sizeof(command), 
            "iptables -t nat -A POSTROUTING -o %s -j MASQUERADE", out_interface);
    
    return execute_command(command);
}

// 设置端口转发
int setup_port_forwarding(int external_port, const char *internal_ip, int internal_port, const char *protocol) {
    char command[256];
    
    printf("设置端口转发: %s:%d -> %s:%d (%s)\n", 
           "0.0.0.0", external_port, internal_ip, internal_port, protocol);
    
    // DNAT 规则
    snprintf(command, sizeof(command), 
            "iptables -t nat -A PREROUTING -p %s --dport %d -j DNAT --to-destination %s:%d",
            protocol, external_port, internal_ip, internal_port);
    
    return execute_command(command);
}

// Ping 主机
int ping_host(const char *host, int count) {
    char command[128];
    snprintf(command, sizeof(command), "ping -c %d %s", count, host);
    printf("Ping 测试: %s\n", host);
    return execute_command(command);
}

// 路由跟踪
int traceroute_host(const char *host) {
    char command[128];
    snprintf(command, sizeof(command), "traceroute %s", host);
    printf("路由跟踪: %s\n", host);
    return execute_command(command);
}

// 检查连通性
int check_connectivity(const char *host, int port) {
    char command[128];
    snprintf(command, sizeof(command), "nc -z -w3 %s %d", host, port);
    printf("连通性检查: %s:%d\n", host, port);
    return execute_command(command);
}

// 打印网络配置
void print_network_config(const network_config_t *config) {
    printf("=== 网络配置 ===\n");
    printf("接口: %s\n", config->interface);
    printf("IP 地址: %s\n", config->ip_addr);
    printf("子网掩码: %s\n", config->netmask);
    printf("网关: %s\n", config->gateway);
    printf("MTU: %d\n", config->mtu);
    printf("================\n");
}

// 打印路由配置
void print_route_config(const route_config_t *route) {
    printf("=== 路由配置 ===\n");
    printf("目标网络: %s\n", route->destination);
    printf("网关: %s\n", route->gateway);
    printf("接口: %s\n", route->interface);
    printf("优先级: %d\n", route->metric);
    printf("================\n");
}

// 打印 iptables 规则
void print_iptables_rule(const iptables_rule_t *rule) {
    printf("=== iptables 规则 ===\n");
    printf("表: %s\n", rule->table);
    printf("链: %s\n", rule->chain);
    printf("动作: %s\n", rule->action);
    printf("协议: %s\n", rule->protocol);
    printf("源地址: %s\n", rule->source);
    printf("目标地址: %s\n", rule->destination);
    printf("入接口: %s\n", rule->interface_in);
    printf("出接口: %s\n", rule->interface_out);
    printf("端口: %d\n", rule->port);
    printf("=====================\n");
}
