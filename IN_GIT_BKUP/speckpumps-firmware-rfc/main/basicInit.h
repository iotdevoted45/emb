/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#ifndef MAIN_BASICINIT_H_
#define MAIN_BASICINIT_H_

#include <string.h>
#include "esp_err.h"
#include "esp_system.h"
#include "esp_spiffs.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "wifiCustomStruct.h"

#define TAG (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__) 
esp_err_t init_system();

#endif /* MAIN_BASICINIT_H_ */
