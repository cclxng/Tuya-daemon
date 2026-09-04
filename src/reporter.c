#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include "system_info.h"
#include "report.h"
#include "tuyalink_core.h"
#include "ubus.h"

void gather_and_send_report(tuya_mqtt_context_t *client)
{
	ubus_stats_t sys_info = {0};
	int ubus_ret = ubus_get_system_info(&sys_info);
	if(ubus_ret!=0) syslog(LOG_WARNING, "Ubus system info unavailable (error code: %d)", ubus_ret);

	float cpu_usage = system_info_get_cpu();

    	network_stats interfaces[MAX_INTERFACE];
    	int if_count = system_info_get_network(interfaces, MAX_INTERFACE);

    	system_report_t report = {
        	.uptime = (long)sys_info.uptime,
        	.total_ram = (unsigned long)sys_info.total_memory,
        	.free_ram = (unsigned long)sys_info.free_memory,
        	.cpu_usage = cpu_usage,
        	.interfaces = interfaces,
        	.count = if_count
    	};
    	char *json = build_report(report);
    	if(json != NULL){
        	int json_ret = tuyalink_thing_property_report(client, NULL, json);
        	if(json_ret < 0)
            		syslog(LOG_ERR, "Failed to send report to Tuya (error code: %d)", json_ret);
        		
		free(json);
    	}
}
