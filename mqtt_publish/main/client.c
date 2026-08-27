/*	MQTT (over TCP) Example

	This example code is in the Public Domain (or CC0 licensed, at your option.)

	Unless required by applicable law or agreed to in writing, this
	software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_mac.h" // esp_base_mac_addr_get
#include "mqtt_client.h"
#include "mdns.h"

static const char *TAG = "PUB";

EventGroupHandle_t mqtt_status_event_group;
#define MQTT_CONNECTED_BIT BIT2

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
	esp_mqtt_event_handle_t event = event_data;
	switch (event->event_id) {
		case MQTT_EVENT_CONNECTED:
			ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
			xEventGroupSetBits(mqtt_status_event_group, MQTT_CONNECTED_BIT);
			break;
		case MQTT_EVENT_DISCONNECTED:
			ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
			xEventGroupClearBits(mqtt_status_event_group, MQTT_CONNECTED_BIT);
			break;
		case MQTT_EVENT_SUBSCRIBED:
			ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_UNSUBSCRIBED:
			ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_PUBLISHED:
			ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_DATA:
			ESP_LOGI(TAG, "MQTT_EVENT_DATA");
			break;
		case MQTT_EVENT_ERROR:
			ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
			break;
		case MQTT_EVENT_BEFORE_CONNECT:
			ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
			break;
		default:
			ESP_LOGW(TAG, "Other event id:%d", event->event_id);
			break;
	}
	return;
}

esp_err_t query_mdns_host(const char * host_name, char *ip)
{
    ESP_LOGD(__FUNCTION__, "Query A: %s", host_name);

    struct esp_ip4_addr addr;
    addr.addr = 0;

    esp_err_t err = mdns_query_a(host_name, 10000,  &addr);
    if(err){
        if(err == ESP_ERR_NOT_FOUND){
            ESP_LOGW(__FUNCTION__, "%s: Host was not found!", host_name);
        } else {
            ESP_LOGE(__FUNCTION__, "Query Failed: %s", esp_err_to_name(err));
        }
        return ESP_FAIL;
    }

    ESP_LOGD(__FUNCTION__, "Query A: %s.local resolved to: " IPSTR, host_name, IP2STR(&addr));
    sprintf(ip, IPSTR, IP2STR(&addr));
    return ESP_OK;
}

void convert_mdns_host(char * from, char * to)
{
    ESP_LOGI(__FUNCTION__, "from=[%s]",from);
    strcpy(to, from);
    char *sp;
    sp = strstr(from, ".local");
    if (sp == NULL) return;

    int _len = sp - from;
    ESP_LOGD(__FUNCTION__, "_len=%d", _len);
    char _from[128];
    strcpy(_from, from);
    _from[_len] = 0;
    ESP_LOGI(__FUNCTION__, "_from=[%s]", _from);

    char _ip[128];
    esp_err_t ret = query_mdns_host(_from, _ip);
    ESP_LOGI(__FUNCTION__, "query_mdns_host=%d _ip=[%s]", ret, _ip);
    if (ret != ESP_OK) return;

    strcpy(to, _ip);
    ESP_LOGI(__FUNCTION__, "to=[%s]", to);
}

void client(void *pvParameters)
{
	ESP_LOGI(TAG, "Start CONFIG_MQTT_BROKER=[%s]", CONFIG_MQTT_BROKER);

	// Create Event Group
	mqtt_status_event_group = xEventGroupCreate();
	configASSERT( mqtt_status_event_group );
	xEventGroupClearBits(mqtt_status_event_group, MQTT_CONNECTED_BIT);

	// Set client id from mac
	uint8_t mac[8];
	ESP_ERROR_CHECK(esp_base_mac_addr_get(mac));
	for(int i=0;i<8;i++) {
		ESP_LOGD(TAG, "mac[%d]=%x", i, mac[i]);
	}
	char client_id[64];
	sprintf(client_id, "pub-%02x%02x%02x%02x%02x%02x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	ESP_LOGI(TAG, "client_id=[%s]", client_id);

	// Resolve mDNS host name
	char ip[128];
	ESP_LOGI(TAG, "CONFIG_MQTT_BROKER=[%s]", CONFIG_MQTT_BROKER);
	convert_mdns_host(CONFIG_MQTT_BROKER, ip);
	ESP_LOGI(TAG, "ip=[%s]", ip);
	char uri[138];
	sprintf(uri, "mqtt://%.60s:%d", ip, CONFIG_MQTT_PORT_TCP);
	ESP_LOGI(TAG, "uri=[%s]", uri);

	// Initialize MQTT configuration structure
	esp_mqtt_client_config_t mqtt_cfg = {
		.broker.address.uri = uri,
		.credentials.client_id = client_id,
		.session.protocol_ver = MQTT_PROTOCOL_V_3_1_1
	};

	esp_mqtt_client_handle_t mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
	esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
	esp_mqtt_client_start(mqtt_client);
	xEventGroupWaitBits(mqtt_status_event_group, MQTT_CONNECTED_BIT, false, true, portMAX_DELAY);
	ESP_LOGI(TAG, "Connected to MQTT Broker");

	char buffer[128];
	while (1) {
		TickType_t nowTick = xTaskGetTickCount();
		int length = sprintf(buffer, "Hello World %"PRIu32, nowTick);
		EventBits_t EventBits = xEventGroupGetBits(mqtt_status_event_group);
		ESP_LOGI(TAG, "EventBits=0x%"PRIx32, EventBits);
		if (EventBits & MQTT_CONNECTED_BIT) {
			int msg_id = esp_mqtt_client_publish(mqtt_client, CONFIG_MQTT_PUB_TOPIC, buffer, length, 1, 0);
			ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
		} else {
			ESP_LOGE(TAG, "Disconnected from MQTT Broker");
			break;
		}
		vTaskDelay((CONFIG_PUBLISH_INTERVAL * 1000) / portTICK_PERIOD_MS);
	} // end while

	// Stop connection
	ESP_LOGI(TAG, "Task Delete");
	esp_mqtt_client_stop(mqtt_client);
	vTaskDelete(NULL);
}
