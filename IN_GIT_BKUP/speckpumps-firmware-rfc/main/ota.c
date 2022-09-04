/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#include "ota.h"

extern const uint8_t server_cert_pem_start[] asm("_binary_s3all_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_s3all_pem_end");

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
	switch (evt->event_id) {
	case HTTP_EVENT_ERROR:
		ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
		break;
	case HTTP_EVENT_ON_CONNECTED:
		ESP_LOGE(TAG, "HTTP_EVENT_ON_CONNECTED");
		break;
	case HTTP_EVENT_HEADER_SENT:
		ESP_LOGE(TAG, "HTTP_EVENT_HEADER_SENT");
		break;
	case HTTP_EVENT_ON_HEADER:
		ESP_LOGE(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
		break;
	case HTTP_EVENT_ON_DATA:
		//ESP_LOGE(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
		break;
	case HTTP_EVENT_ON_FINISH:
		ESP_LOGE(TAG, "HTTP_EVENT_ON_FINISH");
		break;
	case HTTP_EVENT_DISCONNECTED:
		ESP_LOGE(TAG, "HTTP_EVENT_DISCONNECTED");
		break;
	default:
		break;
	}
	return ESP_OK;
}

void task_simple_ota(void *pvParameter)
{
	ESP_LOGI(TAG, "Starting OTA Process ... ");

	mqtt_msg_def msg_to_publish;
	char ota_url[256];


	xEventGroupWaitBits(common_events_group, (1<< enum_start_ota), false, true, portMAX_DELAY);
	memset(ota_url, 0, sizeof(ota_url));
	sprintf(ota_url, "%s", fw_file);
	ESP_LOGI(TAG, "OTA URL = [%s]", ota_url);

	cJSON *root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "resp", "ota_status");
	cJSON_AddStringToObject(root, "msg", "starting OTA");
	cJSON_PrintPreallocated(root, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	cJSON_Delete(root);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);

	esp_http_client_config_t config = {
			.url = ota_url,
			//.cert_pem = (char *)server_cert_pem_start,
			.event_handler = _http_event_handler,
	};


	esp_err_t ret = esp_https_ota(&config);


	cJSON *ota_result = cJSON_CreateObject();
	cJSON_AddStringToObject(ota_result, "resp", "ota_status");

	if (ret == ESP_OK)
	{
		cJSON_AddStringToObject(ota_result, "msg", "OTA Successful, rebooting ...");
		cJSON_PrintPreallocated(ota_result, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
		msg_to_publish.type = enum_ack_msg;
		mqtt_push_to_publish(msg_to_publish);
		ESP_LOGI(TAG, "OTA Successful, rebooting ....");
		vTaskDelay(pdMS_TO_TICKS(1000*10));
		esp_wifi_disconnect();
		esp_wifi_deinit();
		vTaskDelay(pdMS_TO_TICKS(1000*10));
		esp_restart();
	}
	else
	{
		cJSON_AddStringToObject(ota_result, "msg", "OTA failed, rebooting ...");
		cJSON_PrintPreallocated(ota_result, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
		msg_to_publish.type = enum_ack_msg;
		mqtt_push_to_publish(msg_to_publish);
		ESP_LOGE(TAG, "Firmware upgrade failed");
		vTaskDelay(pdMS_TO_TICKS(1000*10));
		esp_wifi_disconnect();
		esp_wifi_deinit();
		vTaskDelay(pdMS_TO_TICKS(1000*10));
		esp_restart();
	}

	while (1)
	{
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
