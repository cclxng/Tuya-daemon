#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H
#define MAX_INTERFACE 5

#include <netinet/in.h>
#include <net/if.h>

typedef struct{
	unsigned long user, nice, system, idle, iowait, irq, softirq, steal;
} cpu_stat_t;

typedef struct{
	char interface[IF_NAMESIZE], ip_address[INET_ADDRSTRLEN], netmask[INET_ADDRSTRLEN];
	unsigned long rx_bytes, tx_bytes;
} network_stats;

long system_info_get_uptime();
void system_info_get_memory(unsigned long *total_ram, unsigned long *free_ram);
int read_cpu(cpu_stat_t *stat);
float system_info_get_cpu();
int system_info_get_network(network_stats *stat, int max_count);

#endif
