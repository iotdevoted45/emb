/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#include "wifiCustomStruct.h"
#include <stdio.h>
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"

#include "fileOperations.h"
#include "basicInit.h"
#include "wifiConnectivity.h"
#include "commondef.h"
#include "rtc_time.h"
#include "mqtt.h"
#include "msg_processor.h"
#include "i2c_peri.h"
#include "ota.h"
#include "modbus.h"

void app_main(void)
{

	ESP_LOGI(TAG, "Initializing System ... ");
	ESP_ERROR_CHECK(init_system());
	
	common_init();
	ESP_ERROR_CHECK(init_file_system());
	ESP_LOGI(TAG, "Initialized file system successfully");


	ESP_ERROR_CHECK(init_wifi());
	ESP_LOGI(TAG, "Initialized wifi-stack successfully");
	ESP_LOGI(TAG, "Checking provisioning data ... ");

	memset((void*)mbus_map_element, 0, sizeof(mbus_map_element_type)*MAX_MBUS_REG);
	read_mbus_map();
	read_mbus_comm_param();
	ESP_LOGI(TAG, "Initializing Modbus driver ... ");
	modbus_master_init();

	memset(&prov_data, 0, sizeof(prov_data_def));
	read_provisioned_data();

	memset(&priming_data, 0, sizeof(priming_data_def));
	read_priming_data();

	init_scheduler();
	read_operating_mode();
	read_device_type();

	update_operating_modes_leds();

	if(ESP_OK != i2c_master_init())
	{
		ESP_LOGE(TAG, "Error in starting i2c Master");
	}
	if(ESP_OK != init_MCP79410())
	{
		ESP_LOGE(TAG, "Error in starting MCP79410");
	}

	if(device_type == ENUM_DEVICE_TYPE_RS485)
	{
		turnoff_motor(ENUM_TRIG_NONE);
	}

	xTaskCreate(&information_task, "Information Task", 6*1024, NULL, 5, NULL);
	xTaskCreate(&task_indication_leds,"led_flsh", 8*1024, NULL, 5, NULL );

	ESP_LOGI(TAG, "[%d][%s][%s]", prov_data.isProvisioned, prov_data.ssid, prov_data.password);
	if(prov_data.isProvisioned == 0)
	{
		ESP_LOGI(TAG, "Device is not provisioned");
		start_provisioning();
	}
	else
	{
		ESP_LOGI(TAG, "Device is provisioned");
		ESP_LOGI(TAG, "Starting in the station mode ... ");
		start_station();
		xTaskCreate(&task_time_sync, "Time sync Task", 8*1024, NULL, 5, NULL);
		xTaskCreate(&task_switch_action, "Switch Scan", 8*1024, NULL, 5, NULL);
		xTaskCreate(&task_timed_action, "Timed_action", 8*1024, NULL, 5, NULL);
		xTaskCreate(&task_sched_start_action, "Sched_start", 8*1024, NULL, 5, NULL);
		xTaskCreate(&task_mqtt_pub_msg_hanle, "Mqtt pub task", 4*1024, NULL, 5, NULL);
		xTaskCreate(&task_mqtt_rcvd_msg_handle, "Mqtt sub task", 8*1024, NULL, 5, NULL);
		xTaskCreate(&task_simple_ota, "OTA_task", 12*1024, NULL, 5, NULL);

		start_mqtt();

	}
	
	while(true)
	{
		vTaskDelay(portMAX_DELAY);
	}

}
