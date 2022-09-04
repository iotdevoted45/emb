/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#ifndef MAIN_I2C_PERI_H_
#define MAIN_I2C_PERI_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"

#include <time.h>
#include <sys/time.h>
#include "esp_sntp.h"

#include "driver/i2c.h"

#include "commondef.h"

esp_err_t i2c_master_init(void);
esp_err_t init_MCP79410();
esp_err_t MCP79410_RTC_getTime(struct tm *time);
esp_err_t MCP79410_RTC_setTime(struct tm *time);


#endif /* MAIN_I2C_PERI_H_ */
