#include <stdio.h>
#include "mqtt_client_interface.h"
#include "tuyalink_core.h"
#include "tuya_cacert.h"
#include "tuya_error_code.h"
#include "tuya_connect.h"
#include <syslog.h>
#include "cJSON.h"
#include <errno.h>
#include <string.h>

static tuya_mqtt_context_t client_instance;

static void on_connected(tuya_mqtt_context_t* context, void* user_data){
	syslog(LOG_INFO, "Connected to Tuya");
}

static void on_disconnect(tuya_mqtt_context_t* context, void* user_data){
    syslog(LOG_INFO, "Disconnectd from Tuya");
}

static void on_messages(tuya_mqtt_context_t* context, void* user_data, const tuyalink_message_t* msg){
    if(msg->type != THING_TYPE_ACTION_EXECUTE) return;

    cJSON *root = cJSON_Parse(msg->data_string);
    cJSON *text = cJSON_GetObjectItem(cJSON_GetObjectItem(root, "inputParams"), "text");

    if (text && cJSON_IsString(text)) {
        FILE *fp = fopen("/tmp/tuya_action.log", "w");
        if (fp) {
            fprintf(fp, "%s\n", text->valuestring);
            fclose(fp);
        } else {
            syslog(LOG_ERR, "fopen failed: %s", strerror(errno));
        }
    }
    cJSON_Delete(root);
}

void tuya_connect_loop(){
	tuya_mqtt_loop(&client_instance);
}

int tuya_connect_init(const char *deviceId, const char *deviceSecret){
	int ret = OPRT_OK;
	tuya_mqtt_context_t* client = &client_instance;

	ret = tuya_mqtt_init(client, &(const tuya_mqtt_config_t) {
        .host = "m1.tuyacn.com",
        .port = 8883,
        .cacert = tuya_cacert_pem,
        .cacert_len = sizeof(tuya_cacert_pem),
        .device_id = deviceId,
        .device_secret = deviceSecret,
        .keepalive = 100,
        .timeout_ms = 2000,
        .on_connected = on_connected,
        .on_disconnect = on_disconnect,
        .on_messages = on_messages
	});
if(ret != 0 ) return ret;
ret = tuya_mqtt_connect(client);

	return ret;
}
int tuya_connect_report(const char *json_payload){
	return tuyalink_thing_property_report(&client_instance, NULL, json_payload);
}

