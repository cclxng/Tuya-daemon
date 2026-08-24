#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include "system_info.h"
#include "report.h"
#include "tuya_connect.h"

void gather_and_send_report(void)
{
    long uptime = system_info_get_uptime();

    unsigned long total_ram, free_ram;
    system_info_get_memory(&total_ram, &free_ram);

    float cpu_usage = system_info_get_cpu();

    network_stats interfaces[MAX_INTERFACE];
    int if_count = system_info_get_network(interfaces, MAX_INTERFACE);

    system_report_t report = {
        .uptime = uptime,
        .total_ram = total_ram,
        .free_ram = free_ram,
        .cpu_usage = cpu_usage,
        .interfaces = interfaces,
        .count = if_count
    };
    char *json = build_report(report);
    if(json != NULL){
        int json_ret = tuya_connect_report(json);
        if(json_ret < 0)
            syslog(LOG_ERR, "Failed to send report to Tuya (error code: %d)", json_ret);
        free(json);
    }
}
