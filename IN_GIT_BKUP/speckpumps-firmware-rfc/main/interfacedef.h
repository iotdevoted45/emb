/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#ifndef MAIN_INTERFACEDEF_H_
#define MAIN_INTERFACEDEF_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"

#define led_mode_rs485		(5)
#define led_mode_pfc		(12)

#define led_auto_manual		(21)

#define led_pfc_stop		(13)
#define led_pfc_n1			(14)
#define led_pfc_n2			(15)
#define led_pfc_n3			(18)

#define contact_pfc_stop	(32)
#define contact_pfc_n1		(27)
#define contact_pfc_n2		(26)
#define contact_pfc_n3		(25)

#define led_mode_clean 		(22)
#define led_wifi			(23)
#define led_server			(33)

#define switch_mode_select		(39)
#define switch_speed_select		(35)
#define switch_flood			(36)

#define rtc_cell_pin		(ADC_CHANNEL_6)

#define I2C_SDA_IO			(2)
#define I2C_SCL_IO			(4)
#define I2C_FREQ			(100000)

#define CLEAN_MODE_TIME		(5)   //sec
#define WIFI_CLEAN_TIME		(10)  //sec

#endif /* MAIN_INTERFACEDEF_H_ */
