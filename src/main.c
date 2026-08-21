#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "tuya_connect.h"
#include "system_info.h"
#include <cJSON.h>
#include <stdlib.h>
#include "report.h"
#include "args.h"
#include <syslog.h>
#include "fork_daemon.h"
#include "signals.h"
#include <ctype.h>

#define REPORT_INTERVAL_SECONDS 5

static void gather_and_send_report(void)
{
    long uptime = system_info_get_uptime();
    printf("Uptime: %ld seconds\n", uptime);

    unsigned long total_ram, free_ram;
    system_info_get_memory(&total_ram, &free_ram);
    printf("Total RAM: %lu MB, Free RAM; %lu MB\n", total_ram / (1024*1024), free_ram / (1024*1024));

    float cpu_usage = system_info_get_cpu();
    printf("CPU usage: %.2f%%\n", cpu_usage);

    network_stats interfaces[MAX_INTERFACE];
    int if_count = system_info_get_network(interfaces, MAX_INTERFACE);
    for(int i=0; i<if_count; i++)
        printf("%s: ip=%s netmask=%s tx=%lu rx=%lu\n",
               interfaces[i].interface, interfaces[i].ip_address, interfaces[i].netmask,
               interfaces[i].tx_bytes, interfaces[i].rx_bytes);

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

int main(int argc, char **argv){
	openlog("tuya daemon", LOG_PID, LOG_DAEMON);
	setup_signal_handler();
	arguments_t args = {0};

	parse_args(argc, argv, &args);

	if(args.device_id == NULL || args.device_secret == NULL || args.product_id == NULL ||
   		isspace((unsigned char)args.device_id[0]) ||
   		isspace((unsigned char)args.device_secret[0]) ||
   		isspace((unsigned char)args.product_id[0])){
    		syslog(LOG_ERR, "Not all arguments imputted");
    		return 1;
		}
	syslog(LOG_INFO, "Starting with product_id=%s", args.product_id);	
	if(args.daemon){
		int dae = fork_daemon();
		if(dae != 0){
			syslog(LOG_ERR, "Error creating daemon");
			return 1;
			}
		}
	
	int ret = tuya_connect_init(args.device_id, args.device_secret);
	if(ret != 0){
		syslog(LOG_ERR, "Connection to Tuya server unsuccessful (error code %d)", ret);
		return 1;
	}
	
	time_t last_report = 0;

	while(keep_running){
    		tuya_connect_loop();
    		time_t now = time(NULL);
    		if(now - last_report >= REPORT_INTERVAL_SECONDS){
        		gather_and_send_report();
        		last_report = now;
    		}
	}

	syslog(LOG_INFO, "Connection with Tuya server was closed");
	closelog();

	return 0;
}
