/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#ifndef MAIN_MQTT_H_
#define MAIN_MQTT_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"


#include "mqtt_client.h"

#include "commondef.h"
#include "fileOperations.h"
#include "wifiConnectivity.h"
#include "basicInit.h"
#include "msg_processor.h"

char mqttMsgPubChannel[MAX_MQTT_TPC_SIZE];

void start_mqtt(void);  
void mqtt_push_to_publish(mqtt_msg_def msg);
void task_mqtt_rcvd_msg_handle(void *pV);
void task_mqtt_pub_msg_hanle(void *pV);

#endif /* MAIN_MQTT_H_ */
