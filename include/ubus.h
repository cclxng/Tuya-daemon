#ifndef UBUS_H
#define UBUS_H

#define MAX_DEVICES 15

#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include <stdint.h>
#include <net/if.h>
#include <netinet/in.h>

enum {
	TOTAL_MEMORY,
	FREE_MEMORY,
	__MEMORY_MAX,
};

enum {
	UPTIME_DATA,
	MEMORY_DATA,
	__INFO_MAX,
};

enum {
	DEVICE_STATISTICS,
	__DEVICE_MAX,
};

enum {
	STATS_RX,
	STATS_TX,
	__STATS_MAX,
};

enum {
	DUMP_INTERFACE,
	__DUMP_MAX,
};

enum {
	L3_DEVICE,
	IPV4_INTERFACE,
	__INTERFACE_MAX,
};

enum {
	IP_ADDRESS,
	IP_NETMASK,
	__IP_MAX,
};

typedef struct {
	uint64_t free_memory, total_memory;
	uint32_t uptime;
} ubus_stats_t;

typedef struct {
	uint64_t tx, rx;
	char interface[IF_NAMESIZE], ip_address[INET_ADDRSTRLEN], netmask[INET_ADDRSTRLEN];
} ubus_network_t;

typedef struct {
	ubus_network_t devices[MAX_DEVICES];
	int count;
} ubus_network_list_t;

static const struct blobmsg_policy memory_policy[__MEMORY_MAX] = {
	[TOTAL_MEMORY] = { .name = "total", .type = BLOBMSG_TYPE_INT64 },
	[FREE_MEMORY] = { .name = "free", .type = BLOBMSG_TYPE_INT64 },
};

static const struct blobmsg_policy info_policy[__INFO_MAX] = {
	[MEMORY_DATA] = { .name = "memory", .type = BLOBMSG_TYPE_TABLE },
	[UPTIME_DATA] = { .name = "uptime", .type = BLOBMSG_TYPE_INT32 },
};

static const struct blobmsg_policy device_policy[__DEVICE_MAX] = {
	[DEVICE_STATISTICS] = { .name = "statistics", .type = BLOBMSG_TYPE_TABLE },
};

static const struct blobmsg_policy stats_policy[__STATS_MAX] = {
	[STATS_RX] = { .name = "rx_bytes", .type = BLOBMSG_TYPE_INT64 },
	[STATS_TX] = { .name = "tx_bytes", .type = BLOBMSG_TYPE_INT64 },
};

static const struct blobmsg_policy dump_policy[__DUMP_MAX] = {
        [DUMP_INTERFACE] = { .name = "interface", .type = BLOBMSG_TYPE_ARRAY },
};

static const struct blobmsg_policy interface_policy[__INTERFACE_MAX] = {
        [L3_DEVICE] = { .name = "l3_device", .type = BLOBMSG_TYPE_STRING },
        [IPV4_INTERFACE] = { .name = "ipv4-address", .type = BLOBMSG_TYPE_ARRAY },
};

static const struct blobmsg_policy ip_policy[__IP_MAX] = {
        [IP_ADDRESS] = { .name = "address", .type = BLOBMSG_TYPE_STRING },
        [IP_NETMASK] = { .name = "mask", .type = BLOBMSG_TYPE_INT32 },
};

int ubus_client_init();
void ubus_client_deinit();
void ubus_lookup_all_objects();
int ubus_get_system_info(ubus_stats_t *out);
int ubus_get_network_info(ubus_network_list_t *out);
#endif
