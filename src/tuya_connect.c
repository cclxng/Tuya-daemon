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

static void on_connected(tuya_mqtt_context_t* context, void* user_data){
	syslog(LOG_INFO, "Connected to Tuya");
}

static void on_disconnect(tuya_mqtt_context_t* context, void* user_data){
    syslog(LOG_INFO, "Disconnectd from Tuya");
}

static void handle_save_text_action(cJSON *root){
    cJSON *text = cJSON_GetObjectItem(cJSON_GetObjectItem(root, "inputParams"), "text");
    if(!(text && cJSON_IsString(text))){
        syslog(LOG_ERR, "save_text action missing text parameter");
        return;
    }
    FILE *fp = fopen("/tmp/tuya_action.log", "w");
    if(!fp){
        syslog(LOG_ERR, "fopen failed: %s", strerror(errno));
        return;
    }
    fprintf(fp, "%s\n", text->valuestring);
    fclose(fp);
}

static void handle_action(cJSON *root){
    cJSON *action_code = cJSON_GetObjectItem(root, "actionCode");
    if(!(action_code && cJSON_IsString(action_code))){
        syslog(LOG_ERR, "Received action with no action_code.");
        return;
    }

    if(strcmp(action_code->valuestring, "save_text") == 0)
        handle_save_text_action(root);
    else 
        syslog(LOG_WARNING, "Unsuported action: %s", action_code->valuestring);
}

static void on_messages(tuya_mqtt_context_t* context, void* user_data, const tuyalink_message_t* msg){
    cJSON *root = cJSON_Parse(msg->data_string);
    if(root == NULL){
        syslog(LOG_ERR, "Failed to parse message from Tuya.");
        return;
    }

    switch (msg->type){
        case THING_TYPE_ACTION_EXECUTE:
        handle_action(root);
        break;
        default:
        syslog(LOG_INFO, "Received unhandled message type: %d", msg->type);
        break;
    }
    cJSON_Delete(root);
}

int tuya_connect_init(tuya_mqtt_context_t *client, const char *deviceId, const char *deviceSecret){
	int ret = OPRT_OK;

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

    if(ret != OPRT_OK ) return ret;
    ret = tuya_mqtt_connect(client);
    if(ret != OPRT_OK){
	    tuya_mqtt_deinit(client);
	    return ret;
    }

	return ret;
}

void tuya_connect_close(tuya_mqtt_context_t *client){
	tuya_mqtt_disconnect(client);
	tuya_mqtt_deinit(client);
}
