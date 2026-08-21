#include <stdio.h>
#include <stdlib.h>
#include "report.h"
#include <errno.h>
#include <syslog.h>
#include <string.h>

static void format_number(char *entry, size_t entry_size, const char *key, long value){
	snprintf(entry, entry_size, "\"%s\":{\"value\":%ld}", key, value);
}

static void format_cpu(char *entry, size_t entry_size, float value){
	snprintf(entry, entry_size, "\"cpu_usage\":{\"value\":%.0f}", value);
}

static void format_interface(char *entry, size_t entry_size, network_stats *interf){
    	snprintf(entry, entry_size, "name=%s;ip=%s;netmask=%s;tx=%lu;rx=%lu",
        interf->interface, interf->ip_address, interf->netmask,
        interf->tx_bytes, interf->rx_bytes);
}

char *build_report(system_report_t report){
	size_t buf_size = 1024;
	char *buf = malloc(buf_size);
	if(buf == NULL){
		syslog(LOG_ERR, "Couldn't allocate memory: %s", strerror(errno));
		return NULL;
	}
	
	char entry[128];
	int offset = 0;
	offset += snprintf(buf + offset, buf_size - offset, "{");

	format_number(entry, sizeof(entry), "uptime", report.uptime);
	offset += snprintf(buf + offset, buf_size - offset, "%s,", entry);

	format_number(entry, sizeof(entry), "total_ram", report.total_ram / (1024*1024));
	offset += snprintf(buf + offset, buf_size - offset, "%s,", entry);

	format_number(entry, sizeof(entry), "free_ram", report.free_ram / (1024*1024));
	offset += snprintf(buf + offset, buf_size - offset, "%s,", entry);

	format_cpu(entry, sizeof(entry), report.cpu_usage);
	offset += snprintf(buf + offset, buf_size - offset, "%s,", entry);

	offset += snprintf(buf + offset, buf_size - offset, "\"network_info\":{\"value\":[");
	for (int i = 0; i < report.count; i++) {
    		format_interface(entry, sizeof(entry), &report.interfaces[i]);
    		offset += snprintf(buf + offset, buf_size - offset, "\"%s\"", entry);
    		if (i < report.count - 1) offset += snprintf(buf + offset, buf_size - offset, ",");
		}
		snprintf(buf + offset, buf_size - offset, "]}}");	
	
	return buf;
}
