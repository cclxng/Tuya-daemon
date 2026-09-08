#include <stdio.h>
#include <unistd.h>
#include "system_info.h"
#include <syslog.h>

int read_cpu(cpu_stat_t *stat){
	FILE *fp = fopen("/proc/stat", "r");
	if(fp == NULL){
		syslog(LOG_ERR, "Couldn't open CPU stats");
		return -1;
	}
	int n = fscanf(fp,"cpu %lu %lu %lu %lu %lu %lu %lu %lu",
		&stat->user, &stat->nice, &stat->system, &stat->idle,
	       	&stat->iowait, &stat->irq, &stat->softirq, &stat->steal);
	fclose(fp);
	if(n!=8){
		syslog(LOG_ERR, "Unexpected /proc/stat format (%d of 8 fields)", n);
		return -1;
	}
	return 0;
}

float system_info_get_cpu(){
	cpu_stat_t st1, st2;

	if(read_cpu(&st1)!=0) return 0.0f;
	sleep(1);
	if(read_cpu(&st2)!=0) return 0.0f;
	
	unsigned long idle_delta = st2.idle - st1.idle;
	unsigned long total1 = st1.user + st1.nice + st1.system + st1.idle + st1.iowait + st1.irq + st1.softirq + st1.steal;
	unsigned long total2 = st2.user + st2.nice + st2.system + st2.idle + st2.iowait + st2.irq + st2.softirq + st2.steal;
	unsigned long total_delta = total2 - total1;

	float usage = (1.0f - (float)idle_delta / total_delta) *100.0f;

	return usage;
	
}

