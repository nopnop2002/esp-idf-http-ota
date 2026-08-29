/*	WiFi station Example

	This example code is in the Public Domain (or CC0 licensed, at your option.)

	Unless required by applicable law or agreed to in writing, this
	software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "mdns.h"

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

#define WIFI_MAXIMUM_RETRY 3

#define WIFI_AP_SSID "ESP32 OTA Update"

static const char *TAG = "WIFI";

static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		if (s_retry_num < WIFI_MAXIMUM_RETRY) {
			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(TAG, "retry to connect to the AP");
		} else {
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
		ESP_LOGI(TAG,"connect to the AP fail");
	} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

esp_err_t wifi_init_sta(char *ssid, char *passwd)
{
	s_wifi_event_group = xEventGroupCreate();

	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_event_handler_instance_t instance_any_id;
	esp_event_handler_instance_t instance_got_ip;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
		ESP_EVENT_ANY_ID,
		&event_handler,
		NULL,
		&instance_any_id));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
		IP_EVENT_STA_GOT_IP,
		&event_handler,
		NULL,
		&instance_got_ip));

#if 0
	wifi_config_t wifi_config = {
		.sta = {
			.ssid = ssid,
			.password = passwd,
			/* Setting a password implies station will connect to all security modes including WEP/WPA.
			 * However these modes are deprecated and not advisable to be used. Incase your Access point
			 * doesn't support WPA2, these mode can be enabled by commenting below line */
			.threshold.authmode = WIFI_AUTH_WPA2_PSK,

			.pmf_cfg = {
				.capable = true,
				.required = false
			},
		},
	};
#endif
	wifi_config_t wifi_config = {0};
	strcpy((char *)wifi_config.sta.ssid, ssid);
	strcpy((char *)wifi_config.sta.password, passwd);
	wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK,
	wifi_config.sta.pmf_cfg.capable = true;
	wifi_config.sta.pmf_cfg.required = true;

	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
	 * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
	esp_err_t ret_value = ESP_OK;
	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
		WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
		pdFALSE,
		pdFALSE,
		portMAX_DELAY);

	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
	 * happened. */
	if (bits & WIFI_CONNECTED_BIT) {
		ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", ssid, passwd);
	} else if (bits & WIFI_FAIL_BIT) {
		ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", ssid, passwd);
		ret_value = ESP_FAIL;
	} else {
		ESP_LOGE(TAG, "UNEXPECTED EVENT");
		ret_value = ESP_FAIL;
	}

	/* The event will not be processed after unregister */
	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
	vEventGroupDelete(s_wifi_event_group);
	return ret_value;
}

void initialize_mdns(char *hostname)
{
	//initialize mDNS
	ESP_ERROR_CHECK( mdns_init() );
	//set mDNS hostname (required if you want to advertise services)
	ESP_ERROR_CHECK( mdns_hostname_set(hostname) );
	ESP_LOGI(TAG, "mdns hostname set to: [%s]", hostname);

	//initialize service
	ESP_ERROR_CHECK( mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0) );

#if 0
	//set default mDNS instance name
	ESP_ERROR_CHECK( mdns_instance_name_set("ESP32 with mDNS") );
#endif
}

esp_err_t wifi_init_ap(void)
{
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	wifi_config_t wifi_config = {
		.ap = {
			.ssid = WIFI_AP_SSID,
			.ssid_len = strlen(WIFI_AP_SSID),
			.channel = 6,
			.authmode = WIFI_AUTH_OPEN,
			.max_connection = 3
		},
	};

	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	return ESP_OK;
}

wifi_mode_t wifi_init(void)
{
	// Initialize NVS
	ESP_LOGI(TAG, "Start");
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	wifi_mode_t wifi_mode = WIFI_MODE_STA;

	// Open NVS
	nvs_handle_t my_handle;
	ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));

	nvs_type_t out_type;
#ifndef CONFIG_MODE
	// Find Key
	ret = nvs_find_key(my_handle, "wifi_mode_ap", &out_type);
	ESP_LOGI(TAG, "nvs_find_key ret=%d", ret);
	if (ret == ESP_OK) {
		wifi_mode = WIFI_MODE_AP;
		// Erase key
		ESP_ERROR_CHECK(nvs_erase_key(my_handle, "wifi_mode_ap"));
	}
#endif

	if (wifi_mode == WIFI_MODE_STA) {
		ESP_LOGI(TAG, "Stating WIFI_MODE_STA");
		char ssid[128];
		char passwd[128];
		char hostname[128];
#ifdef CONFIG_MODE
		strcpy(ssid, CONFIG_ESP_WIFI_SSID);
		strcpy(passwd, CONFIG_ESP_WIFI_PASSWORD);
		strcpy(hostname, CONFIG_MDNS_HOSTNAME);
#else
		size_t ssid_length = sizeof(ssid);
		ESP_ERROR_CHECK(nvs_get_str(my_handle, "wifi_ssid", ssid, &ssid_length));
		ESP_LOGI(TAG, "ssid_length=%d", ssid_length);

		size_t passwd_length = sizeof(passwd);
		ESP_ERROR_CHECK(nvs_get_str(my_handle, "wifi_passwd", passwd, &passwd_length));
		ESP_LOGI(TAG, "passwd_length=%d", passwd_length);

		size_t hostname_length = sizeof(passwd);
		ESP_ERROR_CHECK(nvs_get_str(my_handle, "wifi_hostname", hostname, &hostname_length));
		ESP_LOGI(TAG, "hostname_length=%d", hostname_length);
#endif
		ESP_LOGI(TAG, "ssid=[%s]", ssid);
		ESP_LOGI(TAG, "passwd=[%s]", passwd);
		ESP_LOGI(TAG, "hostname=[%s]", hostname);

		// Initialize WiFi Station
		ret = wifi_init_sta(ssid, passwd);
		ESP_LOGI(TAG, "wifi_init_sta ret=%d", ret);
		if (ret == ESP_OK) {

			// Initialize mDNS
			initialize_mdns(hostname);
	
#ifdef CONFIG_MODE
			// Erase key
			ret = nvs_find_key(my_handle, "wifi_mode_ap", &out_type);
			ESP_LOGI(TAG, "nvs_find_key ret=%d", ret);
			if (ret == ESP_OK) {
				ESP_ERROR_CHECK(nvs_erase_key(my_handle, "wifi_mode_ap"));
			}

			// Write key
			ESP_ERROR_CHECK(nvs_set_str(my_handle, "wifi_ssid", ssid));
			ESP_ERROR_CHECK(nvs_set_str(my_handle, "wifi_passwd", passwd));
			ESP_ERROR_CHECK(nvs_set_str(my_handle, "wifi_hostname", hostname));
			ESP_ERROR_CHECK(nvs_commit(my_handle));
#endif
	
		} else {
#ifndef CONFIG_MODE
			// Use AP mode on the next startup.
			ESP_ERROR_CHECK(nvs_set_i16(my_handle, "wifi_mode_ap", 1));
			ESP_ERROR_CHECK(nvs_commit(my_handle));
#endif
			nvs_close(my_handle);
			ESP_LOGW(TAG, "Station mode initialize fail. Restart Now");
			vTaskDelay(1000);
			esp_restart();
		}

	} // WIFI_MODE_STA 

	if (wifi_mode == WIFI_MODE_AP) {
		ESP_LOGI(TAG, "Stating WIFI_MODE_AP");
		// Initialize WiFi AP
		ret = wifi_init_ap();
		ESP_LOGI(TAG, "wifi_init_ap ret=%d", ret);
	} // WIFI_MODE_AP

	// Close NVS
	nvs_close(my_handle);
	if (wifi_mode == WIFI_MODE_STA) {
		ESP_LOGI(TAG, "Start WIFI_MODE_STA");
	} else {
		ESP_LOGI(TAG, "Start WIFI_MODE_AP");
	}
	return wifi_mode;
}
