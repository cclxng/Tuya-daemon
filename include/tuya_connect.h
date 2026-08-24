#ifndef TUYA_CONNECT_H
#define TUYA_CONNECT_H

int tuya_connect_init(const char *device_id, const char *device_secret);
void tuya_connect_loop();
int tuya_connect_report(const char *json_payload);
void tuya_connect_close();
#endif
