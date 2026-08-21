#include <sys/sysinfo.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "system_info.h"
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <errno.h>

long system_info_get_uptime(){
	struct sysinfo info;
	if(sysinfo(&info) == -1){
		syslog(LOG_ERR, "sysinfo failed: %s", strerror(errno));
		return 0;
	}
	return info.uptime;
}

void system_info_get_memory(unsigned long *total_ram, unsigned long *free_ram){
    struct sysinfo info;
    if(sysinfo(&info) == -1){
        syslog(LOG_ERR, "sysinfo failed: %s", strerror(errno));
        *total_ram = 0;
        *free_ram = 0;
        return;
    }
    *total_ram = info.totalram * info.mem_unit;
    *free_ram = info.freeram * info.mem_unit;
}

int read_cpu(cpu_stat_t *stat){
	FILE *fp = fopen("/proc/stat", "r");
	if(fp == NULL){
		syslog(LOG_ERR, "Couldn't open CPU stats");
		return -1;
	}
	fscanf(fp,"cpu %lu %lu %lu %lu %lu %lu %lu %lu",
		&stat->user, &stat->nice, &stat->system, &stat->idle,
	       	&stat->iowait, &stat->irq, &stat->softirq, &stat->steal);
	fclose(fp);
	return 0;
}

float system_info_get_cpu(){
	cpu_stat_t st1, st2;

	read_cpu(&st1);
	sleep(1);
	read_cpu(&st2);

	unsigned long idle_delta = st2.idle - st1.idle;
	unsigned long total1 = st1.user + st1.nice + st1.system + st1.idle + st1.iowait + st1.irq + st1.softirq + st1.steal;
	unsigned long total2 = st2.user + st2.nice + st2.system + st2.idle + st2.iowait + st2.irq + st2.softirq + st2.steal;
	unsigned long total_delta = total2 - total1;

	float usage = (1.0f - (float)idle_delta / total_delta) *100.0f;

	return usage;
	
}

int system_info_get_network(network_stats *stat, int max_count){
	struct ifaddrs *ifaddr;

	if(getifaddrs(&ifaddr) == -1){
		syslog(LOG_ERR, "getifaddrs failed: %s", strerror(errno));
		return -1;
	}

	int count = 0;

	for(struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next){
		if(ifa->ifa_addr == NULL) continue;
		if(strcmp(ifa->ifa_name, "lo") == 0) continue;
		if(ifa->ifa_addr->sa_family != AF_PACKET) continue;
		if(ifa->ifa_data == NULL) continue;
		if(count >= max_count) continue;
		
		strncpy(stat[count].interface, ifa->ifa_name, IF_NAMESIZE);
		stat[count].ip_address[0] = '\0';
		stat[count].netmask[0] = '\0';
		struct rtnl_link_stats *stats = (struct rtnl_link_stats *)ifa->ifa_data;

		stat[count].tx_bytes = stats->tx_bytes;
		stat[count].rx_bytes = stats->rx_bytes;
		count++;


	}

	for(struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next){
		if(ifa->ifa_addr == NULL) continue;
		if(strcmp(ifa->ifa_name, "lo") == 0) continue;
		if(ifa->ifa_addr->sa_family != AF_INET) continue;
		if(ifa->ifa_netmask == NULL) continue;

		network_stats *entry = NULL;
		for(int i=0; i<count; i++){
			if(strcmp(stat[i].interface, ifa->ifa_name) == 0){
				entry = &stat[i];
				break;
			}
		}
		if (entry == NULL) continue;
		
		struct sockaddr_in *addr = (struct sockaddr_in *) ifa->ifa_addr;
		struct sockaddr_in *netmask = (struct sockaddr_in *) ifa->ifa_netmask;
		
		if(inet_ntop(AF_INET, &addr->sin_addr, entry->ip_address, INET_ADDRSTRLEN) == NULL)
			syslog(LOG_ERR, "inet_ntop failed for interface %s: %s", ifa->ifa_name, strerror(errno));

		if(inet_ntop(AF_INET, &netmask->sin_addr, entry->netmask, INET_ADDRSTRLEN) == NULL)
    			syslog(LOG_ERR, "inet_ntop failed for interface %s: %s", ifa->ifa_name, strerror(errno));
	}

	freeifaddrs(ifaddr);
	return count;
}
