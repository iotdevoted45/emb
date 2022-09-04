/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#include "msg_processor.h"
#include "mqtt.h"
#include "rtc_time.h"
#include "wifiCustomStruct.h"
#include "ota.h"
#include "modbus.h"


const char key_cmd[] = "cmd";
const char key_resp[] = "resp";
const char key_status[] = "status";
const char key_timestamp[] = "time";

const char key_value[] = "value";
const char key_trigger[] = "trigger";

const char key_device_type[] = "device_type";
const char cmd_get_device_type[] = "get_device_type";
const char cmd_set_device_type[] = "set_device_type";

const char key_device_operation_mode[] = "operation_mode";
const char cmd_get_device_operation_mode[] = "get_op_mode";
const char cmd_set_device_operation_mode[] = "set_op_mode";

const char key_timezone[] = "tz";
const char cmd_get_timezone[] = "get_timezone";
const char cmd_set_timezone[] = "set_timezone";

const char key_flood_state[] = "flood_state";
const char cmd_flood_state[] = "get_flood_state";

const char key_pfc_speed[] = "pfc_speed";
const char key_mbus_speed[] = "mbus_speed";
const char key_on_time[] = "on_time";
const char key_remaining_time[] = "remaining_time";
const char cmd_set_motor_speed[] = "set_speed";
const char cmd_get_motor_speed[] = "get_speed";

const char key_scheduler_number[] = "sched";
const char key_scheduler_hh[] = "hh";
const char key_scheduler_mm[] = "mm";
const char key_scheduler_en[] = "enable";
const char key_scheduler_active_days[] = "days";
const char cmd_set_scheduler[] = "set_scheduler";
const char cmd_get_scheduler[] = "get_scheduler";

const char key_voltage[] = "milli_volt";
const char cmd_get_rtc_battery_voltage[] = "get_rtc_voltage";

const char key_baudrate[] = "baud_rate";
const char key_slave_id[] = "slave_id";
const char cmd_set_mbus_comm_parameter[] = "mbus_set_param";
const char cmd_get_mbus_comm_parameter[] = "mbus_get_param";

const char key_id[] = "id";
const char key_address[] = "address";
const char key_enable[] = "enable";
const char key_param[] = "param";
const char cmd_set_mbus_map[] = "mbus_set_map";
const char cmd_get_mbus_map[] = "mbus_get_map";


const char cmd_set_mbus_map_batch[] = "mbus_set_map_batch"; // Added - Harry
const char cmd_get_mbus_map_batch[] = "mbus_get_map_batch"; // Added - Harry
const char key_mbus_map_data[] = "map_data"; // Added - Harry
const char cmd_port_check[] = "port_check"; // Added - Harry

const char key_fw_version[] = "FW_version";
const char cmd_get_fw_version[] = "get_fw_version";

const char cmd_reboot[] = "reboot";

const char key_wifi_ssid[] = "ssid";
const char cmd_get_wifi_ssid[] = "wifi_ssid";

const char cmd_clear_wifi[] = "wifi_clear";

const char key_motor_fault[] = "motor_fault";
const char key_motor_fault_type[] = "motor_fault_type";
const char cmd_check_motor_fault[] = "check_motor_fault";

const char key_ota_file[] = "ota_file";
const char cmd_start_ota[] = "ota_update";

const char key_pwr[] = "power_cons";
const char cmd_power_consumption[] = "get_power_consumption";

const char key_priming_en[] = "pri_enable";
const char key_speed[] = "speed";
const char key_priming_time[] = "pri_time";
const char cmd_get_priming[] = "get_priming";
const char cmd_set_priming[] = "set_priming";

const char cmd_rst_sched[] = "sched_reset";

const char cmd_set_hb_duration[] = "set_heartbeat";
const char cmd_get_hb_duration[] = "get_heartbeat";
const char res_hb[] = "heartbeat";

const char key_active_sched[] = "active_sched";
const char cmd_get_active_sched[] = "get_active_scheduler";
const char res_missed_scheduler[] = "missed_scheduler";


const char key_pri_remaining_time[] = "pri_remaining_time"; // Priming_remaining_time - Harry

void action_get_rtc_battery_voltage(char *);
void action_get_fw_version(char *msg);

void action_get_device_type(char *);
void action_set_device_type(char *);

void action_get_operation_mode(char *);
void action_set_operation_mode(char *);

void action_get_timezone(char *);
void action_set_timezone(char *);

void action_get_flood_state(char *);

void action_set_speed(char *);

void on_set_scheduler(char *);
void on_get_scheduler(char *);

void action_get_mbus_comm_param(char *msg);
void action_set_mbus_comm_param(char *msg);

void action_get_mbus_map(char *msg);
void action_set_mbus_map(char *msg);

void action_reboot(char *msg);
void action_get_wifi_ssid(char *msg);
void action_clear_wifi_cred(char *msg);

void action_start_ota(char *msg);
void action_power_consumption(char *msg);

void action_get_priming(char *msg);
void action_set_priming(char *msg);

void action_sched_rst(char *msg);


cmd_action_def cmd_action_map[] = {
		{(char *)cmd_get_rtc_battery_voltage, action_get_rtc_battery_voltage},
		{(char *)cmd_get_device_type, action_get_device_type},
		{(char *)cmd_set_device_type, action_set_device_type},
		{(char *)cmd_get_device_operation_mode, action_get_operation_mode},
		{(char *)cmd_set_device_operation_mode, action_set_operation_mode},
		{(char *)cmd_get_timezone, action_get_timezone},
		{(char *)cmd_set_timezone, action_set_timezone},
		{(char *)cmd_flood_state, action_get_flood_state},
		{(char *)cmd_set_motor_speed, action_set_speed},
		{(char *)cmd_get_motor_speed, action_get_speed},
		{(char *)cmd_set_scheduler, on_set_scheduler},
		{(char *)cmd_get_scheduler, on_get_scheduler},
		{(char *)cmd_set_mbus_comm_parameter, action_set_mbus_comm_param},
		{(char *)cmd_get_mbus_comm_parameter, action_get_mbus_comm_param},
		{(char *)cmd_set_mbus_map, action_set_mbus_map},
		{(char *)cmd_get_mbus_map, action_get_mbus_map},
		{(char *)cmd_get_fw_version, action_get_fw_version},
		{(char *)cmd_reboot, action_reboot},
		{(char *)cmd_get_wifi_ssid, action_get_wifi_ssid},
		{(char *)cmd_clear_wifi, action_clear_wifi_cred},
		{(char *)cmd_check_motor_fault, action_chek_motor_fault},
		{(char *)cmd_start_ota, action_start_ota},
		{(char *)cmd_power_consumption, action_power_consumption},
		{(char *)cmd_get_priming, action_get_priming},
		{(char *)cmd_set_priming, action_set_priming},
		{(char *)cmd_rst_sched, action_sched_rst},
		{(char *)cmd_set_hb_duration, action_set_heartbeat},
		{(char *)cmd_get_hb_duration, action_get_heartbeat},

		{(char *)cmd_set_mbus_map_batch, action_set_mbus_map_batch},
		{(char *)cmd_get_mbus_map_batch, action_get_mbus_map_batch}, // Harry - Added Callback for Batch Mbus Map

		{NULL, NULL}
};

void process_cmd(char *msg)
{
	cJSON *root;
	root = cJSON_Parse(msg);
	if(!root)
	{
		ESP_LOGE(TAG, "Error in Parsing received MQTT msg");
		return;
	}
	
	if(cJSON_HasObjectItem(root, key_cmd))
	{
		ESP_LOGI(TAG, "Processing :%s \n", msg);
		cmd_action_def *p_map;
		p_map = &cmd_action_map[0];

		while(1)
		{
			if(p_map->cmd == NULL)
			{
				ESP_LOGI(TAG, "Breaking loop of msg processor");
				break;
			}
			if(0 == strcmp(cJSON_GetObjectItem(root, key_cmd)->valuestring, p_map->cmd))
			{
				if(p_map->callback)
				{
					p_map->callback(msg);
					break;
				}
			}

			p_map++;
		}
	}
	cJSON_Delete(root);
}

void action_get_rtc_battery_voltage(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_get_rtc_battery_voltage);
	cJSON_AddNumberToObject(response, key_voltage, get_rtc_battery_voltage());
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);
}

void action_get_device_type(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	cJSON *root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, key_resp, cmd_get_device_type);
	cJSON_AddNumberToObject(root, key_status, 1);
	cJSON_AddNumberToObject(root, key_device_type, device_type);
	cJSON_AddNumberToObject(root, key_timestamp, current_unix_time);
	cJSON_PrintPreallocated(root, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(root);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);
}

void action_set_device_type(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	uint8_t success = 0;
	uint8_t temp_device_type = 0;

	cJSON *rcvd = cJSON_Parse(msg);
	if(cJSON_HasObjectItem(rcvd, key_device_type))
	{
		temp_device_type = cJSON_GetObjectItem(rcvd, key_device_type)->valueint;
		if( ((device_type_def)temp_device_type == ENUM_DEVICE_TYPE_PFC) ||
				((device_type_def)temp_device_type == ENUM_DEVICE_TYPE_RS485)
		)
		{
			device_type = (device_type_def)temp_device_type;
			update_device_type();
			success = 1;
		}
	}
	cJSON_Delete(rcvd);
	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_set_device_type);
	cJSON_AddNumberToObject(response, key_status, success);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_AddNumberToObject(response, key_device_type, device_type);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);

}


void action_get_operation_mode(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_get_device_operation_mode);
	cJSON_AddNumberToObject(response, key_status, 1);
	cJSON_AddNumberToObject(response, key_device_operation_mode, device_operation_mode);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish); 
}

void action_set_operation_mode(char *msg)
{

	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	uint8_t success = 0;
	device_operation_mode_type temp_op_mode;

	cJSON *rcvd = cJSON_Parse(msg);
	if(cJSON_HasObjectItem(rcvd, key_device_operation_mode))
	{
		temp_op_mode = cJSON_GetObjectItem(rcvd, key_device_operation_mode)->valueint;
		if( ((device_operation_mode_type)temp_op_mode == ENUM_OP_MODE_AUTO) ||
				((device_operation_mode_type)temp_op_mode == ENUM_OP_MODE_MANUAL) ||
				((device_operation_mode_type)temp_op_mode == ENUM_OP_MODE_CLEAN)
		)
		{
			device_operation_mode = temp_op_mode;
			update_operating_mode();
			if(device_operation_mode == ENUM_OP_MODE_CLEAN)
			{
				turnoff_motor(ENUM_TRIG_NONE);
			}
			success = 1;
			update_operating_modes_leds();
		}
	}
	cJSON_Delete(rcvd);

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_set_device_operation_mode);
	cJSON_AddNumberToObject(response, key_status, success);
	cJSON_AddNumberToObject(response, key_device_operation_mode, device_operation_mode);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);

}

void action_get_timezone(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_get_timezone);
	cJSON_AddStringToObject(response, key_timezone, device_time_zone.time_zone);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);
}

void action_set_timezone(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	uint8_t success = 0;

	cJSON *rcvd = cJSON_Parse(msg);
	if(cJSON_HasObjectItem(rcvd, key_timezone))
	{
		sprintf(device_time_zone.time_zone, "%s", cJSON_GetObjectItem(rcvd, key_timezone)->valuestring);
		ESP_LOGI(TAG, "timezone = [%s]", device_time_zone.time_zone);
		update_time_zone();
		success = 1;
	}

	cJSON_Delete(rcvd);

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_set_timezone);
	cJSON_AddNumberToObject(response, key_status, success);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);
}

void action_get_flood_state(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_flood_state);
	cJSON_AddNumberToObject(response, key_flood_state, flood_situation);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);
}



void action_set_speed(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	uint8_t success = 0;


	uint16_t pfc_speed = 0;
	uint16_t mbus_speed = 0;
	uint8_t set_value = 0;
	time_t ontime;
	int scheduler_no = 0;


	cJSON *rcvd = cJSON_Parse(msg);
	if(cJSON_HasObjectItem(rcvd, key_value) && cJSON_HasObjectItem(rcvd, key_on_time))
	{
		ontime = (time_t)cJSON_GetObjectItem(rcvd, key_on_time)->valueint;
		set_value = cJSON_GetObjectItem(rcvd, key_value)->valueint;
		device_operation_mode = cJSON_GetObjectItem(rcvd, key_device_operation_mode)->valueint;
		if( (device_type == ENUM_DEVICE_TYPE_PFC) && cJSON_HasObjectItem(rcvd, key_pfc_speed))
		{
			
			pfc_speed = cJSON_GetObjectItem(rcvd, key_pfc_speed)->valueint;
			mbus_speed = 0;
			success = 1;
			motor_port.temp_Value =0 ;
		}
		if( (device_type == ENUM_DEVICE_TYPE_RS485) && cJSON_HasObjectItem(rcvd, key_mbus_speed))
		{
			mbus_speed = cJSON_GetObjectItem(rcvd, key_mbus_speed)->valueint;
			if(set_value){
				if(mbus_speed <= 2){
					motor_port.temp_speed = 0;
					motor_port.temp_Value = 1;
					motor_port.keep_mode_active = 1 ;
					if(ontime > 0 ){
						motor_port.stop_time = current_unix_time + ontime;
						motor_port.on_target = 1;
						motor_port.keep_mode_active = 1;
					}
				}
				else{
					motor_port.temp_Value = 0;
				}	
			}	
			pfc_speed = 0;
			success = 1;
		}
		if(success == 1)
		{
			if(cJSON_GetObjectItem(rcvd, key_scheduler_number))
			{
				scheduler_no = cJSON_GetObjectItem(rcvd, key_scheduler_number)->valueint;
			}
		}
	}
	cJSON_Delete(rcvd);

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_set_motor_speed);
	cJSON_AddNumberToObject(response, key_status, success);
	cJSON_AddNumberToObject(response, key_value, set_value);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	if(success == 1)
	{
		if(set_value == 1 && motor_port.temp_Value == 0)
		{
			if(ontime == 0)
			{
				ontime = 30*24*3600;
			}
			if(scheduler_no != 0)
			{
				turnon_motor(ontime, ENUM_TRIG_SCHED, pfc_speed, mbus_speed);
				motor_port.active_scheduler = scheduler_no;
			}
			else
			{
				motor_port.active_scheduler = 0;
				turnon_motor(ontime, ENUM_TRIG_APP, pfc_speed, mbus_speed);
			}

		}
		else
		{
			if(scheduler_no != 0)
			{
				turnoff_motor(ENUM_TRIG_SCHED);
			}
			else
			{
				turnoff_motor(ENUM_TRIG_APP);
			}

		}
	}
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);

}

void action_get_speed(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);
	uint16_t power_consumption = 0;
	uint16_t speed = 0;

	time_t priming_remaining_time = 0;

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_get_motor_speed);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_AddNumberToObject(response, key_device_operation_mode, device_operation_mode);

	if(device_operation_mode == 2){
		cJSON_AddNumberToObject(response, key_value, 1);
	}else if((device_operation_mode == 1) && (motor_port.active_scheduler == 0) ){
		cJSON_AddNumberToObject(response, key_value, 0);
	}else if( (motor_port.keep_mode_active == 1) || ((motor_port.temp_speed == 0) && (motor_port.temp_Value == 1)) ){
		cJSON_AddNumberToObject(response, key_value,  motor_port.temp_Value);
	}else{ 	
		cJSON_AddNumberToObject(response, key_value,  motor_port.on_actual);
	}
	if(device_type == ENUM_DEVICE_TYPE_PFC)
	{
		cJSON_AddNumberToObject(response, key_pfc_speed, motor_port.target_speed_pfc);
	}
	else
	{
		if( (motor_port.temp_speed == 0) && (motor_port.temp_Value == 1) )
				cJSON_AddNumberToObject(response, key_mbus_speed, motor_port.temp_speed);
		else if(motor_port.HARD_SPEED_Flag){
			time_t current_unix_time, EX_time = 0;
			time(&current_unix_time);
			EX_time = motor_port.stop_time - current_unix_time;
			if(!motor_port.HS_flag2){
				modbus_get_actual_speed(&speed);
				motor_port.Hard_speed = speed ; 
				turnoff_motor(ENUM_TRIG_SWITCH);
				if(EX_time > 0)	
					motor_port.stop_time = current_unix_time + EX_time;
				cJSON_AddNumberToObject(response, key_mbus_speed, 0 );
			}
			else{
				turnon_motor((EX_time>0 ? EX_time : MAX_RUN_TIME*60), ENUM_TRIG_SWITCH, 0, motor_port.Hard_speed);
				cJSON_AddNumberToObject(response, key_mbus_speed, motor_port.Hard_speed );
			}
			motor_port.HS_flag2 = !(motor_port.HS_flag2); 
		}else{		
		modbus_get_actual_speed(&speed);
		cJSON_AddNumberToObject(response, key_mbus_speed, speed);}
	}

	time_t remaining_time = 0;
	if(motor_port.stop_time != 0)
	{
		remaining_time = motor_port.stop_time-current_unix_time;
	}
	cJSON_AddNumberToObject(response, key_remaining_time, remaining_time);
	cJSON_AddNumberToObject(response, key_trigger, motor_port.trigger);
	cJSON_AddNumberToObject(response, key_active_sched, motor_port.active_scheduler);
	if(device_type == ENUM_DEVICE_TYPE_RS485)
	{
		if(ESP_OK != modbus_get_power_info(&power_consumption)){
			ESP_LOGE(TAG, "Error in reading power consumption data");
			cJSON_AddNumberToObject(response, key_pwr, power_consumption);
		}else{
			cJSON_AddNumberToObject(response, key_pwr, power_consumption);
		}
	}else{
		cJSON_AddNumberToObject(response, key_pwr, power_consumption);
	}

	if(motor_port.priming_time_stop != 0)
	{
		priming_remaining_time = motor_port.priming_time_stop-current_unix_time;
	}

	cJSON_AddNumberToObject(response, key_pri_remaining_time, priming_remaining_time);

	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);
}

void on_set_scheduler(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	uint8_t scheduler_number = 0;
	uint8_t scheduler_hh = 0;
	uint8_t scheduler_mm = 0;
	uint16_t scheduler_motor_start_stop = 0;
	uint8_t scheduler_day = 0;
	uint16_t scheduler_pfc_speed = 0;
	uint16_t scheduler_rs485_speed = 0;
	uint16_t scheduler_en = 0;
	uint8_t success = 0;

	sched_def scheduler;

	cJSON *rcvd = cJSON_Parse(msg);

	if(cJSON_HasObjectItem(rcvd, key_scheduler_number) &&
			cJSON_HasObjectItem(rcvd, key_scheduler_en) &&
			cJSON_HasObjectItem(rcvd, key_scheduler_active_days) &&
			cJSON_HasObjectItem(rcvd, key_scheduler_hh) &&
			cJSON_HasObjectItem(rcvd, key_scheduler_mm) &&
			cJSON_HasObjectItem(rcvd, key_value)
	)
	{
		scheduler_number = cJSON_GetObjectItem(rcvd, key_scheduler_number)->valueint;
		scheduler_en = cJSON_GetObjectItem(rcvd, key_scheduler_en)->valueint;
		scheduler_day = cJSON_GetObjectItem(rcvd, key_scheduler_active_days)->valueint;
		scheduler_hh = cJSON_GetObjectItem(rcvd, key_scheduler_hh)->valueint;
		scheduler_mm = cJSON_GetObjectItem(rcvd, key_scheduler_mm)->valueint;
		scheduler_motor_start_stop = cJSON_GetObjectItem(rcvd, key_value)->valueint;
		if(cJSON_HasObjectItem(rcvd, key_pfc_speed) && (device_type == ENUM_DEVICE_TYPE_PFC))
		{
			scheduler_pfc_speed = cJSON_GetObjectItem(rcvd, key_pfc_speed)->valueint;
			scheduler_rs485_speed = 0;
			success = 1;
		}
		if(cJSON_HasObjectItem(rcvd, key_mbus_speed) && (device_type == ENUM_DEVICE_TYPE_RS485))
		{
			scheduler_pfc_speed = 0;
			scheduler_rs485_speed = cJSON_GetObjectItem(rcvd, key_mbus_speed)->valueint;
			success = 1;
		}

		if( (scheduler_number < 1) || (scheduler_number > MAX_SCHED))
		{
			success = 0;
		}

	}
	cJSON_Delete(rcvd);

	if(success == 1)
	{
		scheduler.enable = scheduler_en;
		scheduler.day = scheduler_day;
		scheduler.start_time_HH = scheduler_hh;
		scheduler.start_time_MM = scheduler_mm;
		scheduler.start_stop_value = scheduler_motor_start_stop;
		scheduler.speed_mbus = scheduler_rs485_speed;
		scheduler.speed_pfc = scheduler_pfc_speed;
		scheduler_update(scheduler_number, scheduler);
	}

	cJSON *response = cJSON_CreateObject();

	cJSON_AddStringToObject(response, key_resp, cmd_set_scheduler);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_AddNumberToObject(response, key_scheduler_en, scheduler_en);
	cJSON_AddNumberToObject(response, key_scheduler_active_days, scheduler_day);
	cJSON_AddNumberToObject(response, key_scheduler_hh, scheduler_hh);
	cJSON_AddNumberToObject(response, key_scheduler_mm, scheduler_mm);
	cJSON_AddNumberToObject(response, key_pfc_speed, scheduler_pfc_speed);
	cJSON_AddNumberToObject(response, key_mbus_speed, scheduler_rs485_speed);
	cJSON_AddNumberToObject(response, key_value, scheduler_motor_start_stop);
	cJSON_AddNumberToObject(response, key_scheduler_number, scheduler_number);
 	
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);
}

void on_get_scheduler(char *msg)
{

	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	uint8_t scheduler_number = 0;
	uint8_t success = 0;

	sched_def scheduler;

	cJSON *rcvd = cJSON_Parse(msg);
	if(cJSON_HasObjectItem(rcvd, key_scheduler_number))
	{
		scheduler_number = cJSON_GetObjectItem(rcvd, key_scheduler_number)->valueint;
		if( (scheduler_number >= 1) && (scheduler_number <= MAX_SCHED))
			success = 1;
	}
	cJSON_Delete(rcvd);

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_get_scheduler);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_AddNumberToObject(response, key_status, success);
	if(success == 1)
	{
		scheduler_read(scheduler_number, &scheduler);
		cJSON_AddNumberToObject(response, key_scheduler_number, scheduler_number);
		cJSON_AddNumberToObject(response, key_scheduler_en, scheduler.enable);
		cJSON_AddNumberToObject(response, key_scheduler_active_days, scheduler.day);
		cJSON_AddNumberToObject(response, key_scheduler_hh, scheduler.start_time_HH);
		cJSON_AddNumberToObject(response, key_scheduler_mm, scheduler.start_time_MM);
		cJSON_AddNumberToObject(response, key_value, scheduler.start_stop_value);
		if(device_type == ENUM_DEVICE_TYPE_PFC)
		{
			cJSON_AddNumberToObject(response, key_pfc_speed, scheduler.speed_pfc);
		}
		else if(device_type == ENUM_DEVICE_TYPE_PFC)
		{
			cJSON_AddNumberToObject(response, key_mbus_speed, scheduler.speed_mbus);
		}
	}
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);
}

void action_get_mbus_comm_param(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_get_mbus_comm_parameter);
	cJSON_AddNumberToObject(response, key_baudrate, mbus_comm_parameter.baud_rate);
	cJSON_AddNumberToObject(response, key_slave_id, mbus_comm_parameter.slave_id);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);
}

void action_set_mbus_comm_param(char *msg)
{

	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	uint8_t success = 0;
	uint32_t baudrate = 0;
	uint8_t slave_id = 0;

	cJSON *rcvd = cJSON_Parse(msg);
	if(cJSON_HasObjectItem(rcvd, key_baudrate) && cJSON_HasObjectItem(rcvd, key_slave_id))
	{
		success = 1;
		baudrate = cJSON_GetObjectItem(rcvd, key_baudrate)->valueint;
		slave_id = cJSON_GetObjectItem(rcvd, key_slave_id)->valueint;
	}
	cJSON_Delete(rcvd);

	mbus_comm_parameter.baud_rate = baudrate;
	mbus_comm_parameter.slave_id = slave_id;
	save_mbus_comm_param();

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_set_mbus_comm_parameter);
	cJSON_AddNumberToObject(response, key_status, success);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);

}

void action_get_mbus_map(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	uint8_t success = 0;
	uint8_t id = 0;

	cJSON *rcvd = cJSON_Parse(msg);
	if(cJSON_HasObjectItem(rcvd, key_id))
	{
		id = cJSON_GetObjectItem(rcvd, key_id)->valueint;
		if(id < MAX_MBUS_REG)
		{
			success = 1;
		}
	}
	cJSON_Delete(rcvd);

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_get_mbus_map);
	cJSON_AddNumberToObject(response, key_status, success);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	if(success == 1)
	{

		cJSON_AddNumberToObject(response, key_id, id);
		cJSON_AddNumberToObject(response, key_address, mbus_map_element[id].addr);
		cJSON_AddNumberToObject(response, key_enable, mbus_map_element[id].enable);
		cJSON_AddNumberToObject(response, key_param, mbus_map_element[id].parameter);

	}

	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	cJSON_Delete(response);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);
}

void action_set_mbus_map(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	uint8_t success = 0;

	uint8_t id = 0;
	uint16_t addr = 0;
	uint16_t param = 0;
	uint8_t enable = 0;

	cJSON *rcvd = cJSON_Parse(msg);
	if(cJSON_HasObjectItem(rcvd, key_id) &&
			cJSON_HasObjectItem(rcvd, key_address) &&
			cJSON_HasObjectItem(rcvd, key_enable) &&
			cJSON_HasObjectItem(rcvd, key_param))
	{
		id = cJSON_GetObjectItem(rcvd, key_id)->valueint;
		addr = cJSON_GetObjectItem(rcvd, key_address)->valueint;
		enable = cJSON_GetObjectItem(rcvd, key_enable)->valueint;
		param = cJSON_GetObjectItem(rcvd, key_param)->valueint;
		if(id < MAX_MBUS_REG)
			success = 1;
	}
	cJSON_Delete(rcvd);

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_set_mbus_map);
	cJSON_AddNumberToObject(response, key_id, id);
	cJSON_AddNumberToObject(response, key_status, success);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	if(success == 1)
	{
		mbus_map_element[id].addr = addr;
		mbus_map_element[id].enable = enable;
		mbus_map_element[id].parameter = param;
		save_mbus_map();
	}

	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	cJSON_Delete(response);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);
}


#if 1
	// Changes Harry - Modbus Map Batch handlers

	// cmd_set_mbus_map_batch

void action_get_mbus_map_batch(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	uint8_t success = 0;

	// Temp
	success = 1;

	cJSON *map_data_array = cJSON_CreateArray();

	cJSON *map_data = cJSON_CreateObject();

	for(int i=0; i < MAX_MBUS_REG; i++){
		
		cJSON_AddNumberToObject(map_data, key_id, i);
		cJSON_AddNumberToObject(map_data, key_address, mbus_map_element[i].addr);
		cJSON_AddNumberToObject(map_data, key_enable, mbus_map_element[i].enable);
		cJSON_AddNumberToObject(map_data, key_param, mbus_map_element[i].parameter);

		cJSON_AddItemToArray(map_data_array, map_data);

		// char* test_str_resp = cJSON_Print(map_data);	
		// printf("Map testing %d,%s,%d", i, test_str_resp,strlen(test_str_resp));

		// cJSON_Delete(map_data);

		// char* test_str = cJSON_Print(map_data_array);	
		// printf("Map testing Array: %s,%d",test_str,strlen(test_str));
	}

	// cJSON *map_data = cJSON_CreateObject();
	// cJSON_AddNumberToObject(map_data, key_id, 1);
	// cJSON_AddNumberToObject(map_data, key_address, mbus_map_element[1].addr);
	// cJSON_AddNumberToObject(map_data, key_enable, mbus_map_element[1].enable);
	// cJSON_AddNumberToObject(map_data, key_param, mbus_map_element[1].parameter);

	// cJSON_AddItemToArray(map_data_array, map_data);

	// cJSON *map_data_2 = cJSON_CreateObject();
	// cJSON_AddNumberToObject(map_data_2, key_id, 2);
	// cJSON_AddNumberToObject(map_data_2, key_address, mbus_map_element[2].addr);
	// cJSON_AddNumberToObject(map_data_2, key_enable, mbus_map_element[2].enable);
	// cJSON_AddNumberToObject(map_data_2, key_param, mbus_map_element[2].parameter);

	// cJSON_AddItemToArray(map_data_array, map_data_2);

	char* test_str = cJSON_Print(map_data_array);	
	printf("Map testing Array: %s,%d",test_str,strlen(test_str));

	// cJSON_Delete(map_data);


	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_get_mbus_map_batch);
	cJSON_AddNumberToObject(response, key_status, success);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_AddItemToObject(response, key_mbus_map_data, map_data_array);


	

	// char* test_str_resp = cJSON_Print(response);	
	// printf("Map testing %s,%d",test_str_resp,strlen(test_str_resp));
	

	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	cJSON_Delete(response);
	// cJSON_Delete(map_data);
	cJSON_Delete(map_data_array);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);
}

void action_set_mbus_map_batch(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	uint8_t success = 0;

	uint8_t id = 0;
	// uint16_t addr = 0;
	// uint16_t param = 0;
	// uint8_t enable = 0;

	cJSON *rcvd = cJSON_Parse(msg);
	cJSON *items = cJSON_GetObjectItem(rcvd, key_mbus_map_data);

	for(int i=0; i<MAX_MBUS_REG; i++){
		cJSON * subitem = cJSON_GetArrayItem(items, i);
		if(	
			cJSON_HasObjectItem(subitem, key_id) &&
			cJSON_HasObjectItem(subitem, key_address) &&
			cJSON_HasObjectItem(subitem, key_enable) &&
			cJSON_HasObjectItem(subitem, key_param)
		){
			id = cJSON_GetObjectItem(subitem, key_id)->valueint;
			mbus_map_element[id].addr = cJSON_GetObjectItem(subitem, key_address)->valueint;
			mbus_map_element[id].enable = cJSON_GetObjectItem(subitem, key_enable)->valueint;
			mbus_map_element[id].parameter = cJSON_GetObjectItem(subitem, key_param)->valueint;
			save_mbus_map();
			success=1;
		}else{
			success=0;
			break;
		}
	}
	cJSON_Delete(rcvd);	
	cJSON_Delete(items);

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_set_mbus_map_batch);
	cJSON_AddNumberToObject(response, key_status, success);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);

	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);

	cJSON_Delete(response);	
}
#endif

void action_get_fw_version(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_get_fw_version);
	cJSON_AddStringToObject(response, key_fw_version, FW_VERSION);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);
}

void action_reboot(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	ESP_LOGI(TAG, "Rebooting wifi section");
	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_reboot);
	cJSON_AddNumberToObject(response, key_status, 1);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);

	cJSON *state = cJSON_CreateObject();
	cJSON_AddStringToObject(state, "Status", "Offline");
	cJSON_PrintPreallocated(state, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	cJSON_Delete(state);
	msg_to_publish.type = enum_status_msg;
	mqtt_push_to_publish(msg_to_publish);

	vTaskDelay(pdMS_TO_TICKS(5000));
	esp_restart();
}

void action_get_wifi_ssid(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	ESP_LOGI(TAG, "Get-Wifi data command received");
	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_get_wifi_ssid);
	cJSON_AddStringToObject(response, key_wifi_ssid, prov_data.ssid);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);
}


void action_clear_wifi_cred(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	ESP_LOGI(TAG, "Clearing wifi credential in device");
	clear_provisioned_data();
	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_clear_wifi);
	cJSON_AddNumberToObject(response, key_status, 1);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);

}

void action_chek_motor_fault(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	motor_err_def motor_err;

	cJSON *response = cJSON_CreateObject();


	cJSON_AddStringToObject(response, key_resp, cmd_check_motor_fault);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);

	if(device_type == ENUM_DEVICE_TYPE_RS485)
	{
		check_for_motor_err(&motor_err);
		cJSON *fault_arry = cJSON_CreateIntArray(motor_err.error_no, 6);
		cJSON *fault_val = cJSON_CreateIntArray(motor_err.value, 6);
		cJSON_AddItemToObject(response, key_motor_fault, fault_arry);
		cJSON_AddItemToObject(response, key_motor_fault_type, fault_val);
		cJSON_AddNumberToObject(response, key_status, 1);
	}
	else
	{
		cJSON_AddNumberToObject(response, key_status, 0);
	}
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);
}

void action_power_consumption(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	uint16_t power_consumption;

	cJSON *response = cJSON_CreateObject();


	cJSON_AddStringToObject(response, key_resp, cmd_power_consumption);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);

	if(device_type == ENUM_DEVICE_TYPE_RS485)
	{
		if(ESP_OK != modbus_get_power_info(&power_consumption))
		{
			ESP_LOGE(TAG, "Error in reading power consumption data");
			cJSON_AddNumberToObject(response, key_pwr, power_consumption);
			cJSON_AddNumberToObject(response, key_status, 0);
		}
		else
		{
			cJSON_AddNumberToObject(response, key_pwr, power_consumption);
			cJSON_AddNumberToObject(response, key_status, 1);
		}
	}
	else
	{
		cJSON_AddNumberToObject(response, key_status, 0);
	}
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);
}

void action_port_check(char *msg){
	mqtt_msg_def msg_to_publish;

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_port_check);
	cJSON_AddNumberToObject(response, key_status, 1);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);
	vTaskDelay(pdMS_TO_TICKS(1000));
	motor_port.action_port_check = false;
}

void action_start_ota(char *msg)
{
	cJSON *response = cJSON_Parse(msg);
	if(cJSON_HasObjectItem(response, key_ota_file))
	{
		memset(fw_file, 0, sizeof(fw_file));
		strcpy(fw_file, cJSON_GetObjectItem(response, key_ota_file)->valuestring);
	}
	else
	{
		return;
	}
	cJSON_Delete(response);
	ESP_LOGI(TAG, "FW file = %s", fw_file);
	xEventGroupSetBits(common_events_group, 1 << enum_start_ota);
}


void action_get_priming(char *msg)
{
	mqtt_msg_def msg_to_publish;
	time_t current_unix_time;
	uint8_t status = 1;

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_get_priming);
	cJSON_AddNumberToObject(response, key_status, status);
	cJSON_AddNumberToObject(response, key_enable, priming_data.is_enabled);
	cJSON_AddNumberToObject(response, key_speed, priming_data.priming_speed);
	cJSON_AddNumberToObject(response, key_priming_time, priming_data.priming_time);
	time(&current_unix_time);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);

}
void action_set_priming(char *msg)
{
	mqtt_msg_def msg_to_publish;
	time_t current_unix_time;
	uint8_t status = 0;

	time_t priming_time;
	uint8_t priming_en;
	uint16_t priming_speed;

	cJSON *rcvd = cJSON_Parse(msg);
	if(rcvd)
	{
		if(cJSON_HasObjectItem(rcvd, key_priming_en) &&
				cJSON_HasObjectItem(rcvd, key_priming_time) &&
				cJSON_HasObjectItem(rcvd, key_speed)

		)
		{
			priming_en = cJSON_GetObjectItem(rcvd, key_priming_en)->valueint;
			priming_time = (time_t)cJSON_GetObjectItem(rcvd, key_priming_time)->valueint;
			priming_speed = cJSON_GetObjectItem(rcvd, key_speed)->valueint;
			status = 1;
			priming_data.is_enabled = priming_en;
			priming_data.priming_time = priming_time;
			priming_data.priming_speed = priming_speed;
			update_priming_data();
		}
		cJSON_Delete(rcvd);
	}
	else
	{
		return;
	}

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_set_priming);
	cJSON_AddNumberToObject(response, key_status, status);
	time(&current_unix_time);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);

}

void action_sched_rst(char *msg)
{
	mqtt_msg_def msg_to_publish;
	time_t current_unix_time;
	uint8_t status = 1;

	motor_port.force_stop = 0;


	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_rst_sched);
	cJSON_AddNumberToObject(response, key_status, status);
	time(&current_unix_time);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	msg_to_publish.type = enum_ack_msg;
	mqtt_push_to_publish(msg_to_publish);
}

void send_hb()
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, res_hb);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	mqtt_push_to_publish(msg_to_publish);
}

void action_set_heartbeat(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	uint8_t success = 0;

	uint8_t enable_hb = 0;
	int hb_interval_sec = 0;


	cJSON *rcvd = cJSON_Parse(msg);
	if ( cJSON_HasObjectItem(rcvd, key_enable)  && cJSON_HasObjectItem(rcvd, key_value) )
	{
		enable_hb = cJSON_GetObjectItem(rcvd, key_enable)->valueint;
		hb_interval_sec = cJSON_GetObjectItem(rcvd, key_value)->valueint;
		success = 1;

	}
	cJSON_Delete(rcvd);

	if(success == 1)
	{
		ESP_LOGI(TAG, "HB enable = %d, interval = %d", enable_hb, hb_interval_sec);
		hb_data.enable = enable_hb;
		hb_data.interval = hb_interval_sec;
		update_hb_data();
	}

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_set_hb_duration);
	cJSON_AddNumberToObject(response, key_status, success);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	mqtt_push_to_publish(msg_to_publish);

}

void action_get_heartbeat(char *msg)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, cmd_get_hb_duration);
	cJSON_AddNumberToObject(response, key_status, 1);
	cJSON_AddNumberToObject(response, key_enable, hb_data.enable);
	cJSON_AddNumberToObject(response, key_value, hb_data.interval);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	mqtt_push_to_publish(msg_to_publish);
}

void missed_scheduler(uint8_t schedule_number)
{
	mqtt_msg_def msg_to_publish;

	time_t current_unix_time;
	time(&current_unix_time);

	cJSON *response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, key_resp, res_missed_scheduler);
	cJSON_AddNumberToObject(response, key_scheduler_number, schedule_number);
	cJSON_AddNumberToObject(response, key_device_operation_mode, device_operation_mode);
	cJSON_AddNumberToObject(response, key_timestamp, current_unix_time);
	cJSON_PrintPreallocated(response, msg_to_publish.msg, sizeof(msg_to_publish.msg), 0);
	ESP_LOGI(TAG, "%s", msg_to_publish.msg);
	cJSON_Delete(response);
	mqtt_push_to_publish(msg_to_publish);
}

//===================================================================================
void task_switch_action(void *pv)
{

	time_t current_unix_time;
	time_t soft_timer_clean_mode;
	time_t soft_timer_wifi_reset;

	uint8_t sw1_prvs_state = 1;
	uint8_t sw2_prvs_state = 1;
	uint8_t fs_prvs_state = 1;

	switch_status.sw1_pressed = 0;
	switch_status.sw2_pressed = 0;

	uint8_t sw1_current_state;
	uint8_t sw2_current_state;
	uint8_t fs_current_state;

	device_operation_mode_type prv_operation_mode = ENUM_OP_MODE_MANUAL;

	xEventGroupWaitBits(common_events_group, 1 << enum_time_synced, false, true, portMAX_DELAY);

	time(&soft_timer_clean_mode);
	time(&soft_timer_wifi_reset);

	while(1)
	{
		time(&current_unix_time);
		sw1_current_state = gpio_get_level(switch_mode_select);
		sw2_current_state = gpio_get_level(switch_speed_select);
		fs_current_state = gpio_get_level(switch_flood);

		if(xEventGroupGetBits(common_events_group) & (1 << enum_connected_to_mqtt))
		{
			//ESP_LOGE(TAG, "Flood gpio = %d %d\r\n", fs_current_state, flood_situation);
			if( (fs_current_state != fs_prvs_state) && (fs_current_state == 0) )
			{
				ESP_LOGI("--->", "Flood");
				ESP_LOGE(TAG, "Flood gpio = %d %d\r\n", fs_current_state, flood_situation);
				flood_situation = 1;
				action_get_flood_state(NULL);
			}

			if( (fs_current_state != fs_prvs_state) && (fs_current_state == 1) )
			{
				ESP_LOGE(TAG, "Flood gpio = %d %d\r\n", fs_current_state, flood_situation);
				ESP_LOGI("--->", "Flood");
				flood_situation = 0;
				action_get_flood_state(NULL);
			}


			fs_prvs_state = fs_current_state;
		}
		if((sw1_current_state == 0) && (sw2_current_state == 0))
		{
			if( (current_unix_time-soft_timer_wifi_reset) > WIFI_CLEAN_TIME)
			{
				ESP_LOGE("-->", "Erasing wifi setting");
				clear_provisioned_data();
				ESP_LOGI(TAG, "Rebooting ....");
				vTaskDelay(pdMS_TO_TICKS(10*1000));
				esp_restart();
			}
			soft_timer_clean_mode = current_unix_time;
		}
		else
		{
			soft_timer_wifi_reset = current_unix_time;

			if( (sw1_current_state != sw1_prvs_state) && (sw1_current_state == 1) && (device_operation_mode != ENUM_OP_MODE_CLEAN))
			{
				if(switch_status.sw1_pressed == 0)
				{
					switch_status.sw1_pressed = 1;
				}
			}

			if( (sw2_current_state != sw2_prvs_state) && (sw2_current_state == 1) && (device_operation_mode != ENUM_OP_MODE_CLEAN))
			{
				if(switch_status.sw2_pressed == 0)
				{
					switch_status.sw2_pressed = 1;
				}
			}

			if( (sw1_current_state == 0) && (sw1_current_state == sw1_prvs_state))
			{
				if( (current_unix_time-soft_timer_clean_mode) > CLEAN_MODE_TIME)
				{
					soft_timer_clean_mode = current_unix_time;
					if(device_operation_mode == ENUM_OP_MODE_CLEAN)
					{
						device_operation_mode = prv_operation_mode;
						update_operating_mode();
						update_operating_modes_leds();
						action_get_operation_mode(NULL);
						do
						{
							vTaskDelay(100);
							sw1_current_state = gpio_get_level(switch_mode_select);
						}while(sw1_current_state == 0);
						switch_status.sw1_pressed = 0;
						sw1_prvs_state = sw1_current_state;
						continue;
					}
					else
					{
						prv_operation_mode = device_operation_mode;
						turnoff_motor(ENUM_TRIG_NONE);
						device_operation_mode = ENUM_OP_MODE_CLEAN;

					}
					update_operating_modes_leds();
					action_get_operation_mode(NULL);
					if(device_operation_mode == ENUM_OP_MODE_MANUAL)
					{
						action_get_speed(NULL);
					}
				}
			}
			else
			{
				soft_timer_clean_mode = current_unix_time;
			}
		}

		sw1_prvs_state = sw1_current_state;
		sw2_prvs_state = sw2_current_state;

		if( (switch_status.sw1_pressed == 1)  )
		{
			ESP_LOGI(TAG, "Switch 1 is pressed");
			if(device_operation_mode == ENUM_OP_MODE_AUTO)
			{
				device_operation_mode = ENUM_OP_MODE_MANUAL;
				motor_port.force_stop = 0;
				motor_port.active_scheduler = 0;
				if( (motor_port.target_speed_pfc != 0) && (device_type == ENUM_DEVICE_TYPE_PFC))
				{
					action_get_speed(NULL);
				}
				if( (motor_port.target_speed_mbus != 0) && (device_type == ENUM_DEVICE_TYPE_RS485))
				{
					action_get_speed(NULL);
				}
			}
			else if(device_operation_mode == ENUM_OP_MODE_MANUAL)
			{
				device_operation_mode = ENUM_OP_MODE_AUTO;
				motor_port.force_stop = 0;
			}
			else
			{
				device_operation_mode = ENUM_OP_MODE_MANUAL;
			}
			update_operating_mode();
			update_operating_modes_leds();
			action_get_operation_mode(NULL);
			if(device_operation_mode == ENUM_OP_MODE_MANUAL)
			{
				action_get_speed(NULL);
			}
			switch_status.sw1_pressed = 0;
		}

		if( switch_status.sw2_pressed == 1)
		{
			ESP_LOGI(TAG, "Switch 2 is pressed");
			if(ENUM_OP_MODE_AUTO == device_operation_mode)
			{
				turnoff_motor(ENUM_TRIG_SWITCH);
			}

			if(ENUM_OP_MODE_MANUAL == device_operation_mode)
			{
				if(ENUM_DEVICE_TYPE_RS485 == device_type)
				{
					motor_port.HARD_SPEED_Flag = true ; 
					action_get_speed(NULL);
					motor_port.HARD_SPEED_Flag = false ; 
				}
				if(ENUM_DEVICE_TYPE_PFC == device_type)
				{
					ESP_LOGI(TAG, "Speed = %d", motor_port.target_speed_pfc);
					switch(motor_port.target_speed_pfc)
					{

					case 0:
						turnon_motor(MAX_RUN_TIME*60, ENUM_TRIG_SWITCH, 1, 0);
						break;
					case 1:
						turnon_motor(MAX_RUN_TIME*60, ENUM_TRIG_SWITCH, 2, 0);
						break;
					case 2:
						turnon_motor(MAX_RUN_TIME*60, ENUM_TRIG_SWITCH, 3, 0);
						break;
					case 3:
						turnon_motor(MAX_RUN_TIME*60, ENUM_TRIG_SWITCH, 0, 0);
						break;
					default:
						turnoff_motor(ENUM_TRIG_SWITCH);
						break;
					}
				}
			}

			switch_status.sw2_pressed = 0;
		}

		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

