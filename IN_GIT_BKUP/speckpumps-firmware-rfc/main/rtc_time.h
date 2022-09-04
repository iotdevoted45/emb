/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#ifndef MAIN_RTC_TIME_H_
#define MAIN_RTC_TIME_H_


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

#include "commondef.h"

void task_time_sync(void *pv);
void set_timezone(char *timezone);
struct tm get_local_time();



#endif /* MAIN_RTC_TIME_H_ */
