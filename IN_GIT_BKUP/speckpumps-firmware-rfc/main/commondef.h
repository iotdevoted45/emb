/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#ifndef MAIN_COMMONDEF_H_
#define MAIN_COMMONDEF_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "nvs.h"

#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "interfacedef.h"
#include "version.h"


#define MAX_MQTT_MSG_SIZE	512
#define MAX_MQTT_TPC_SIZE	256

#define MAX_MBUS_REG		11
#define MAX_SCHED			32
#define MAX_RUN_TIME		(1440) //minutes 24 hrs
#define TAG (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__) 


#define ADC_DEFAULT_VREF    1100
#define ADC_NO_OF_SAMPLES   64

EventGroupHandle_t common_events_group;
SemaphoreHandle_t file_operation_mutex;
SemaphoreHandle_t mutex_mqtt_msg_queue_push;
SemaphoreHandle_t mutex_mbus;
SemaphoreHandle_t mutex_i2c;
SemaphoreHandle_t mutex_adc_reading;
QueueHandle_t queue_mqtt_msg_to_server;
QueueHandle_t queue_mqtt_msg_from_server;

esp_adc_cal_characteristics_t *adc_chars;

typedef enum _event_type_
{
	enum_ap_on,
	enum_wifi_connected,
	enum_wifi_disconnected,
	enum_wifi_connecting,
	enum_time_synced,
	enum_connected_to_internet,
	enum_connected_to_mqtt,
	enum_start_ota,
}common_event_bit_type;

typedef enum _mqtt_msg_
{
	enum_rcvd_msg,
	enum_status_msg,
	enum_info_msg,
	enum_ack_msg
}mqtt_msg_type;

typedef struct __time_zone_
{
	char time_zone[32];
}time_zone_type;

typedef struct __mqtt_msg__
{
	char msg[1024];
	mqtt_msg_type type;
}mqtt_msg_def;


typedef struct __cmd_action__
{
	char *cmd;

	void (*callback) (char *);
}cmd_action_def;

typedef struct __sched_struct__
{
	uint8_t enable;
	uint8_t day;
	uint8_t start_time_HH;
	uint8_t start_time_MM;
	uint32_t speed_pfc;
	uint32_t speed_mbus;
	uint8_t start_stop_value;
}sched_def;

typedef union _word_
{
	uint16_t word;
	uint8_t  byte[2];
}mbus_reg_type;

typedef enum __mbus_map_index_
{
	ENUM_MBUS_REG_START_STOP = 0,
	ENUM_MBUS_REG_TAR_SPEED = 1,
	ENUM_MBUS_REG_ACT_SPEED = 2,
	ENUM_MBUS_REG_OP_MODE = 3,
	ENUM_MBUS_REG_FR_1 = 4,
	ENUM_MBUS_REG_FR_2 = 5,
	ENUM_MBUS_REG_FR_3 = 6,
	ENUM_MBUS_REG_FR_4 = 7,
	ENUM_MBUS_REG_FR_5 = 8,
	ENUM_MBUS_REG_FR_6 = 9,
	ENUM_MBUS_REG_PWR = 10,
}mbus_map_index;

typedef enum __mbus_motor_on_off_enum_
{
	ENUM_MOTOR_ON_0_1 = 1,
	ENUM_MOTOR_ON_1_0 = 2,
	ENUM_MOTOR_ON_DIS = 3,
}mbus_on_off_enum_type;

typedef struct __mbus_map_element__
{
	uint16_t addr;
	uint8_t enable;
	uint16_t parameter;
}mbus_map_element_type;

mbus_map_element_type mbus_map_element[MAX_MBUS_REG];

typedef struct __mbus_comm_parameter_type__
{
	uint16_t slave_id;
	uint32_t baud_rate;
}mbus_comm_parameter_type;

mbus_comm_parameter_type mbus_comm_parameter;

typedef struct __MCP79410RTC__
{
	uint8_t sec;
	uint8_t min;
	uint8_t hrs;
	uint8_t wday;
	uint8_t mday;
	uint8_t mon;
	uint8_t yrs;
}MCP79410RTC_def;

typedef struct __switch_states_
{
	uint8_t sw1_pressed;
	uint8_t sw2_pressed;
}input_sw_type;

typedef enum _motor_trigger_
{
	ENUM_TRIG_NONE,
	ENUM_TRIG_SWITCH,
	ENUM_TRIG_TIMER,
	ENUM_TRIG_APP,
	ENUM_TRIG_SCHED
}motor_trigger_def;

typedef struct __speed_port_sched__
{
	uint8_t on_target;
	uint8_t on_actual;
	uint8_t force_update;
	uint16_t target_speed_mbus;
	uint16_t target_speed_pfc;
	time_t stop_time;
	uint8_t force_stop;
	motor_trigger_def trigger;
	time_t priming_time_stop;
	uint8_t speed_change;
	uint8_t active_scheduler;
	uint16_t Hard_speed;
	uint16_t temp_speed;			// by SHane hold the remain time while motor on on Zero RPM
	uint16_t temp_Value;			// by 		"			"			"			"
	uint8_t keep_mode_active ;		// by 		"			"			"			"
	bool action_port_check;			// by SHane give signale to server to checdk next task 
	bool HARD_SPEED_Flag ;
	bool HS_flag2 ;
}motor_port_def;


typedef enum __device_operation_mode_
{
	ENUM_OP_MODE_NONE = 0,
	ENUM_OP_MODE_AUTO = 1,
	ENUM_OP_MODE_MANUAL =2,
	ENUM_OP_MODE_CLEAN = 3,
}device_operation_mode_type;

typedef enum __device_type_def__
{
	ENUM_DEVICE_TYPE_PFC,
	ENUM_DEVICE_TYPE_RS485,
}device_type_def;

typedef struct __motor_error_def__
{
	int error_no[6];
	int value[6];
	int prv_value[6];
}motor_err_def;

typedef struct __priming_data_def__
{
	uint8_t is_enabled;
	uint16_t priming_speed;
	time_t priming_time;
}priming_data_def;

typedef enum __indication_led_states_
{
	ENUM_LED_STATE_1,
	ENUM_LED_STATE_2,
	ENUM_LED_STATE_3,
	ENUM_LED_STATE_4,
	ENUM_LED_STATE_5,
	ENUM_LED_STATE_6,
	ENUM_LED_STATE_7,
	ENUM_LED_STATE_8,
	ENUM_LED_STATE_9,
	ENUM_LED_STATE_10

}led_state_enum;

typedef enum __wifi_led_states__
{
	ENUM_WIFI_LED_NO_PROV,
	ENUM_WIFI_LED_WIFI_ERR,
	ENUM_WIFI_LED_WIFI_OK,
}wifi_led_states_enum;

typedef enum __server_led_states__
{
	ENUM_SERVER_LED_NO_PROV,
	ENUM_SERVER_NOT_CONNECTED,
	ENUM_SERVER_IN_OTA,
	ENUM_SERVER_CONNECTED,
}server_led_states_enum;

typedef enum __n1_led_state__
{
	ENUM_N1_LED_OFF,
	ENUM_N1_LED_ON,
	ENUM_N1_LED_FLASH,
	ENUM_N1_LED_SCHED,
}n1_led_state_enum;

typedef enum __n2_led_state__
{
	ENUM_N2_LED_OFF,
	ENUM_N2_LED_ON,
	ENUM_N2_LED_FLASH,
	ENUM_N2_LED_SCHED
}n2_led_state_enum;

typedef enum __n3_led_state__
{
	ENUM_N3_LED_OFF,
	ENUM_N3_LED_ON,
	ENUM_N3_LED_FLASH,
	ENUM_N3_LED_SCHED
}n3_led_state_enum;

typedef enum __stop_led_state__
{
	ENUM_NS_LED_OFF,
	ENUM_NS_LED_ON,
}ns_led_state_enum;

typedef enum __rs485_led_state__
{
	ENUM_RS485_LED_OFF,
	ENUM_RS485_LED_FLASH,
	ENUM_RS485_LED_ON,
}led_rs485_state_enum;

typedef struct __hb_data__
{
	int interval;
	uint8_t enable;
}hb_data_type;

uint8_t rs_485_comm_err;

device_operation_mode_type device_operation_mode;
device_type_def device_type;
motor_port_def motor_port;
uint8_t flood_situation;
priming_data_def priming_data;

time_zone_type device_time_zone;
input_sw_type switch_status;
hb_data_type hb_data;

uint8_t scheduler_executed[MAX_SCHED];

void common_init(void);
void information_task(void *pv);
int get_rtc_battery_voltage();


void update_operating_modes_leds();

void turnon_motor(time_t on_time, motor_trigger_def trigger, uint16_t pfc_speed, uint16_t rs485_speed);
void turnoff_motor(motor_trigger_def trigger);

void check_for_motor_err(motor_err_def *err);

void task_timed_action(void *pv);
void task_sched_start_action(void *pv);
void task_indication_leds(void *pv);

#endif /* MAIN_COMMONDEF_H_ */
