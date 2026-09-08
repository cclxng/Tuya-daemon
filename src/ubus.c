#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include <syslog.h>
#include "ubus.h"
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>

static struct ubus_context *ctx;
static uint32_t system_id, network_interface_id, network_device_id;
static bool system_id_ok, network_interface_id_ok, network_device_id_ok;

int ubus_client_init(){
	ctx = ubus_connect(NULL);
	if(!ctx) {
		syslog(LOG_ERR,"Failed to connect to ubus");
		return UBUS_STATUS_CONNECTION_FAILED;
	}
	syslog(LOG_INFO, "Connected to ubus");
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
	syslog(LOG_INFO, "ubus objects resolved: system=%s network.device=%s network.interface=%s",
		system_id_ok ? "yes" : "no",
		network_device_id_ok ? "yes" : "no",
		network_interface_id_ok ? "yes" : "no");
}

static void system_info_cb(struct ubus_request *req, int type, struct blob_attr *msg){
	ubus_stats_t *out = req->priv;
	struct blob_attr *tb[__INFO_MAX];
	struct blob_attr *mem_tb[__MEMORY_MAX];

	int ret = blobmsg_parse(info_policy, __INFO_MAX, tb, blobmsg_data(msg), blobmsg_data_len(msg));
	if(ret!=0) syslog(LOG_WARNING,"Could not parse system info reply");
	if(tb[UPTIME_DATA]) out->uptime = blobmsg_get_u32(tb[UPTIME_DATA]);

	if(tb[MEMORY_DATA]) {
		ret = blobmsg_parse(memory_policy, __MEMORY_MAX, mem_tb, blobmsg_data(tb[MEMORY_DATA]), blobmsg_data_len(tb[MEMORY_DATA]));
		if(ret!=0) syslog(LOG_WARNING,"Could not parse memory table");
		if(mem_tb[TOTAL_MEMORY]) out->total_memory = blobmsg_get_u64(mem_tb[TOTAL_MEMORY]);
		if(mem_tb[FREE_MEMORY]) out->free_memory = blobmsg_get_u64(mem_tb[FREE_MEMORY]);
	}

}

int ubus_get_system_info(ubus_stats_t *out){
	if(!system_id_ok){
		syslog(LOG_WARNING, "Skipping system info: object id not resolved");
		return UBUS_STATUS_NOT_FOUND;
	}
	return ubus_invoke(ctx, system_id, "info", NULL, system_info_cb, out, 2000);
}

static void network_info_cb(struct ubus_request *req, int type, struct blob_attr *msg) {
	ubus_network_list_t *out = req->priv;
	struct blob_attr *dev[__DEVICE_MAX];
	struct blob_attr *stats[__STATS_MAX];
	struct blob_attr *cur;
	int rem;
	
	blobmsg_for_each_attr(cur, msg, rem){
		if(out->count >= MAX_DEVICES) break;
		const char *name = blobmsg_name(cur);
		if(strcmp(name, "lo") == 0) continue;
		blobmsg_parse(device_policy, __DEVICE_MAX, dev, blobmsg_data(cur), blobmsg_data_len(cur));
		if(!dev[DEVICE_STATISTICS]) continue;

		blobmsg_parse(stats_policy, __STATS_MAX, stats, blobmsg_data(dev[DEVICE_STATISTICS]), blobmsg_data_len(dev[DEVICE_STATISTICS]));

		ubus_network_t *d = &out->devices[out->count];
		strncpy(d->interface, name, IF_NAMESIZE - 1);
		if(stats[STATS_RX]) d->rx = blobmsg_get_u64(stats[STATS_RX]);
		if(stats[STATS_TX]) d->tx = blobmsg_get_u64(stats[STATS_TX]);
		out->count++;
		}

}

static void cidr_to_dotted(int32_t prefix_len, char *str){
	uint32_t mask = prefix_len ? htonl(UINT32_MAX << (32 - prefix_len)) : 0;
	struct in_addr addr = { .s_addr = mask };
	inet_ntop(AF_INET, &addr, str, INET_ADDRSTRLEN);
}

static void interface_info_cb(struct ubus_request *req, int type, struct blob_attr *msg) {
	ubus_network_list_t *out = req->priv;
	struct blob_attr *dump[__DUMP_MAX];
	struct blob_attr *interface[__INTERFACE_MAX];
	struct blob_attr *ip[__IP_MAX];
	struct blob_attr *cur, *addr;
	int rem, rem2;

	blobmsg_parse(dump_policy, __DUMP_MAX, dump, blobmsg_data(msg), blobmsg_data_len(msg));
	if(ret != 0) syslog(LOG_WARNING, "Could not parse network.interface dump reply");
	if(!dump[DUMP_INTERFACE]) return;

	blobmsg_for_each_attr(cur, dump[DUMP_INTERFACE], rem){
		blobmsg_parse(interface_policy, __INTERFACE_MAX, interface, blobmsg_data(cur), blobmsg_data_len(cur));
		if(!interface[L3_DEVICE] || !interface[IPV4_INTERFACE]) continue;

		const char *l3dev = blobmsg_get_string(interface[L3_DEVICE]);

		ubus_network_t *d = NULL;
		for(int i=0; i< out->count; i++){
			if(strcmp(l3dev, out->devices[i].interface) == 0){
				d=&out->devices[i];
				break;
				}
			}
	if(!d) continue;

	blobmsg_for_each_attr(addr, interface[IPV4_INTERFACE], rem2){
		blobmsg_parse(ip_policy, __IP_MAX, ip, blobmsg_data(addr), blobmsg_data_len(addr));
		if(!ip[IP_ADDRESS] || !ip[IP_NETMASK]) continue;

		strncpy(d->ip_address, blobmsg_get_string(ip[IP_ADDRESS]), INET_ADDRSTRLEN - 1);
		cidr_to_dotted(blobmsg_get_u32(ip[IP_NETMASK]), d->netmask);
				break;
		}
	
	}
}

int ubus_get_network_info(ubus_network_list_t *out){
	if(!network_device_id_ok){
		syslog(LOG_WARNING, "Skipping network info: device object id not resolved");
		return UBUS_STATUS_CONNECTION_FAILED;
	}

	int ret = ubus_invoke(ctx, network_device_id, "status", NULL, network_info_cb, out, 2000);
	if(ret != 0){
		syslog(LOG_WARNING, "network.device status failed: %s", ubus_strerror(ret));
		return ret;
	}

	if(!network_interface_id_ok){
		syslog(LOG_WARNING, "Skipping IP info: interface object id not resolved");
		return 0;
	}

	ret = ubus_invoke(ctx, network_interface_id, "dump", NULL, interface_info_cb, out, 2000);
	if(ret != 0)
		syslog(LOG_WARNING, "network.interface dump failed: %s", ubus_strerror(ret));

	return ret;
}
