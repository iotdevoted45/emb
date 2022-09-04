/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#ifndef MAIN_WIFICONNECTIVITY_H_
#define MAIN_WIFICONNECTIVITY_H_

#include "esp_err.h"
#include "esp_system.h"
#include "esp_spiffs.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "cJSON.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "basicInit.h"
#include "fileOperations.h"

char ip_address_str[32];

esp_err_t init_wifi();
esp_err_t start_provisioning();
esp_err_t start_station();
esp_err_t get_rssi_info(uint8_t *);

#endif /* MAIN_WIFICONNECTIVITY_H_ */
