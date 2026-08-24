#ifndef TUYA_CONNECT_H
#define TUYA_CONNECT_H
#include "tuyalink_core.h"

int tuya_connect_init(tuya_mqtt_context_t *client, const char *deviceId, const char *deviceSecret);
void tuya_connect_close(tuya_mqtt_context_t *client);

#endif