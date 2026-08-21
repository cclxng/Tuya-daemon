#ifndef REPORT_H
#define REPORT_H

#include "system_info.h"

typedef struct{
	long uptime;
	unsigned long total_ram, free_ram;
	float cpu_usage;
	network_stats *interfaces;
	int count;
} system_report_t;

char *build_report(system_report_t report);

#endif
