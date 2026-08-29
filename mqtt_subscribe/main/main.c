#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_ota_ops.h"
#include "esp_log.h"

static const char *TAG = "MAIN";

wifi_mode_t wifi_init();
esp_err_t http_server_init(void);
void client(void *pvParameters);

void app_main(void)
{
	// Initialize WiFi
	wifi_mode_t wifi_mode = wifi_init();

	// Initialize http server
	ESP_ERROR_CHECK(http_server_init());

	// Mark current app as valid
	const esp_partition_t *partition = esp_ota_get_running_partition();
	ESP_LOGI(TAG, "Currently running partition: %s", partition->label);

	esp_ota_img_states_t ota_state;
	if (esp_ota_get_state_partition(partition, &ota_state) == ESP_OK) {
		if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
			esp_ota_mark_app_valid_cancel_rollback();
		}
	}

	// Start client task
	if (wifi_mode == WIFI_MODE_STA) {
		xTaskCreate(&client, "CLIENT", 1024*4, NULL, 5, NULL);
	}

	while(1) vTaskDelay(10);
}
