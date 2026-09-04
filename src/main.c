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
#include "reporter.h"
#include "ubus.h"

#define REPORT_INTERVAL_SECONDS 5

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
			return 2;
			}
		}

	tuya_mqtt_context_t client;
	
	int ret = tuya_connect_init(&client, args.device_id, args.device_secret);
	if(ret != 0){
		syslog(LOG_ERR, "Connection to Tuya server unsuccessful (error code %d)", ret);
		return 3;
	}

	ubus_client_init();
	ubus_lookup_all_objects();
	
	time_t last_report = 0;

	while(keep_running){
    		tuya_mqtt_loop(&client);
    		time_t now = time(NULL);
    		if(now - last_report >= REPORT_INTERVAL_SECONDS){
        		gather_and_send_report(&client);
        		last_report = now;
    		}
	}
	tuya_connect_close(&client);
	ubus_client_deinit();
	syslog(LOG_INFO, "Connection with Tuya server was closed");
	closelog();

	return 0;
}
