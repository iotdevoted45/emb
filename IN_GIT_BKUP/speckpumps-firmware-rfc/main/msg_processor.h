/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#ifndef MAIN_MSG_PROCESSOR_H_
#define MAIN_MSG_PROCESSOR_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"


#include "cJSON.h"

#include "commondef.h"
#include "fileOperations.h"
#include "wifiConnectivity.h"
#include "basicInit.h"

void process_cmd(char *msg);
void action_get_speed(char *msg);
void action_chek_motor_fault(char *msg);

void task_switch_action(void *pv);

void send_hb();
void action_port_check(char *msg);
void action_set_heartbeat(char *msg);
void action_get_heartbeat(char *msg);
void missed_scheduler(uint8_t schedule_number);


// Mbus Batch Handler - Harry
void action_get_mbus_map_batch(char *msg);
void action_set_mbus_map_batch(char *msg);

#endif /* MAIN_MSG_PROCESSOR_H_ */
