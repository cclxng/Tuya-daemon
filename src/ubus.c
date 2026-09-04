#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include <syslog.h>
#include "ubus.h"
#include <stdbool.h>

static struct ubus_context *ctx;
static uint32_t system_id, network_interface_id, network_device_id;
static bool system_id_ok, network_interface_id_ok, network_device_id_ok;

int ubus_client_init(){
	ctx = ubus_connect(NULL);
	if(!ctx) {
		syslog(LOG_ERR,"Failed to connect to ubus");
		return 1;
	}
	return 0;
}

void ubus_client_deinit(){
	ubus_free(ctx);
	ctx = NULL;
}

static int ubus_lookup_object(struct ubus_context *ctx, const char *path, uint32_t *id){
	int ret = ubus_lookup_id(ctx, path, id);
    	if (ret != 0)
		syslog(LOG_ERR, "Failed to look up ubus object '%s': %s", path, ubus_strerror(ret));
    	return ret;
}

void ubus_lookup_all_objects(){
    	system_id_ok = (ubus_lookup_object(ctx, "system", &system_id) == 0);
    	network_interface_id_ok = (ubus_lookup_object(ctx, "network.interface", &network_interface_id) == 0);
    	network_device_id_ok = (ubus_lookup_object(ctx, "network.device", &network_device_id) == 0);
}

static void system_info_cb(struct ubus_request *req, int type, struct blob_attr *msg){
	ubus_stats_t *out = req->priv;
	struct blob_attr *tb[__INFO_MAX];
	struct blob_attr *mem_tb[__MEMORY_MAX];

	int ret = blobmsg_parse(info_policy, __INFO_MAX, tb, blobmsg_data(msg), blobmsg_data_len(msg));
	if(ret!=0) syslog(LOG_WARNING,"Could not parse data");
	
	if(tb[UPTIME_DATA]) out->uptime = blobmsg_get_u32(tb[UPTIME_DATA]);

	if(tb[MEMORY_DATA]) {
		ret = blobmsg_parse(memory_policy, __MEMORY_MAX, mem_tb, blobmsg_data(tb[MEMORY_DATA]), blobmsg_data_len(tb[MEMORY_DATA]));
		if(ret!=0) syslog(LOG_WARNING,"Could not parse data");
		if(mem_tb[TOTAL_MEMORY]) out->total_memory = blobmsg_get_u64(mem_tb[TOTAL_MEMORY]);
		if(mem_tb[FREE_MEMORY]) out->free_memory = blobmsg_get_u64(mem_tb[FREE_MEMORY]);
	}

}

int ubus_get_system_info(ubus_stats_t *out){
	if(!system_id_ok){
		syslog(LOG_WARNING, "Skipping system info: object id not resolved");
		return 1;
	}
	return ubus_invoke(ctx, system_id, "info", NULL, system_info_cb, out, 2000);
}
