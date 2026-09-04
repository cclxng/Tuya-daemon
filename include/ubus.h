#ifndef UBUS_H
#define UBUS_H

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

typedef struct {
	uint64_t free_memory, total_memory;
	uint32_t uptime;
} ubus_stats_t;

typedef struct {
	uint64_t tx, rx;
	char interface[IF_NAMESIZE], ip_address[INET_ADDRSTRLEN], netmask[INET_ADDRSTRLEN];
} ubus_network_t;

static const struct blobmsg_policy memory_policy[__MEMORY_MAX] = {
	[TOTAL_MEMORY] = { .name = "total", .type = BLOBMSG_TYPE_INT64 },
	[FREE_MEMORY] = { .name = "free", .type = BLOBMSG_TYPE_INT64 },
};

static const struct blobmsg_policy info_policy[__INFO_MAX] = {
	[MEMORY_DATA] = { .name = "memory", .type = BLOBMSG_TYPE_TABLE },
	[UPTIME_DATA] = { .name = "uptime", .type = BLOBMSG_TYPE_INT32 },
};

int ubus_client_init();
void ubus_client_deinit();
void ubus_lookup_all_objects();
int ubus_get_system_info(ubus_stats_t *out);

#endif
