/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#ifndef MAIN_OTA_H_
#define MAIN_OTA_H_

#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

#include "commondef.h"
#include "mqtt.h"

#define OTA_BASE_URL "https://firmwarevms.s3.amazonaws.com/"

char fw_file[256];

void task_simple_ota(void *pvParameter);


#endif /* MAIN_OTA_H_ */
