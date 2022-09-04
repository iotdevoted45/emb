/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#include "commondef.h"
#include "rtc_time.h"
#include "fileOperations.h"
#include "msg_processor.h"
#include "modbus.h"

esp_err_t _motor_off();

void _gpio_input(uint8_t pin)
{
	gpio_pad_select_gpio(pin);
	gpio_set_direction(pin, GPIO_MODE_INPUT);
}

void _gpio_output(uint8_t pin, uint8_t value)
{
	gpio_pad_select_gpio(pin);
	gpio_set_direction(pin, GPIO_MODE_INPUT_OUTPUT);
	gpio_set_level(pin, value);
}

void common_init(void)
{
	common_events_group = xEventGroupCreate();

	xEventGroupClearBits(common_events_group, 1 << enum_ap_on);
	xEventGroupClearBits(common_events_group, 1 << enum_wifi_connected);
	xEventGroupClearBits(common_events_group, 1 << enum_wifi_disconnected);
	xEventGroupClearBits(common_events_group, 1 << enum_wifi_connecting);
	xEventGroupClearBits(common_events_group, 1 << enum_time_synced);
	xEventGroupClearBits(common_events_group, 1 << enum_connected_to_internet);
	xEventGroupClearBits(common_events_group, 1 << enum_connected_to_mqtt);
	xEventGroupClearBits(common_events_group, 1 << enum_start_ota);

	file_operation_mutex = xSemaphoreCreateMutex();
	mutex_mqtt_msg_queue_push = xSemaphoreCreateMutex();
	mutex_mbus = xSemaphoreCreateMutex();
	mutex_i2c = xSemaphoreCreateMutex();
	mutex_adc_reading = xSemaphoreCreateMutex();

	queue_mqtt_msg_from_server = xQueueCreate(5, sizeof(mqtt_msg_def));
	queue_mqtt_msg_to_server = xQueueCreate(10, sizeof(mqtt_msg_def));

	motor_port.on_target = 0;
	motor_port.on_actual = 0;
	motor_port.force_stop = 0;
	motor_port.trigger = ENUM_TRIG_NONE;
	motor_port.keep_mode_active = 0;
	motor_port.action_port_check = 0;
	motor_port.HS_flag2 = 0;
	motor_port.HARD_SPEED_Flag = 0;
	_gpio_output(led_mode_rs485, 0);
	_gpio_output(led_auto_manual, 0);
	_gpio_output(led_mode_clean, 0);
	_gpio_output(led_mode_rs485, 0);
	_gpio_output(led_mode_pfc, 0);

	_gpio_output(led_pfc_n1, 0); _gpio_output(contact_pfc_n1, 0);
	_gpio_output(led_pfc_n2, 0); _gpio_output(contact_pfc_n2, 0);
	_gpio_output(led_pfc_n3, 0); _gpio_output(contact_pfc_n3, 0);
	vTaskDelay(pdMS_TO_TICKS(500));
	_gpio_output(led_pfc_stop, 1); _gpio_output(contact_pfc_stop, 1);

	_gpio_output(led_wifi, 0);
	_gpio_output(led_server, 0);

	_gpio_input(switch_mode_select);
	_gpio_input(switch_speed_select);
	_gpio_input(switch_flood);

	flood_situation = 0;
	rs_485_comm_err = 0;

	_motor_off();

	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_config_channel_atten(rtc_cell_pin, ADC_ATTEN_DB_11);
	adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, ADC_DEFAULT_VREF, adc_chars);

}

void information_task(void *pv)
{
	uint16_t old_speed = 0;
	uint16_t new_speed = 0;
	
	while(1)
	{

		ESP_LOGI(TAG, "Free memory: %d bytes,  Device_Mode:%d, Device_Type:%d", esp_get_free_heap_size(), device_operation_mode, device_type);
#if 1	//	Changed by SHane for published msg from Device without calling from mobile 
		
		if(device_type == ENUM_DEVICE_TYPE_RS485){
			modbus_get_actual_speed(&new_speed);
			int change = new_speed - old_speed;
			if((change >= 5) || (change <= -5)){
				ESP_LOGI(TAG,"Published Speed due to change in Speed");
				action_get_speed(NULL);
			}else{
				// D0 nothing 
			}
			old_speed = new_speed;
		}
#endif		

		vTaskDelay(pdMS_TO_TICKS(5000));
	}
}

int get_rtc_battery_voltage()
{
	xSemaphoreTake(mutex_adc_reading, portMAX_DELAY);
	uint32_t adc_reading = 0;
	for (int i = 0; i < ADC_NO_OF_SAMPLES; i++)
		adc_reading += adc1_get_raw((adc1_channel_t)rtc_cell_pin);
	int voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars)/ADC_NO_OF_SAMPLES;
	ESP_LOGI(TAG, "Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
	xSemaphoreGive(mutex_adc_reading);
	return voltage;
}

void update_operating_modes_leds()
{
	/*
	switch(device_operation_mode)
	{
	case ENUM_OP_MODE_AUTO:
		gpio_set_level(led_auto_manual, 0);
		gpio_set_level(led_mode_clean, 0);
		break;
	case ENUM_OP_MODE_MANUAL:
		gpio_set_level(led_auto_manual, 1);
		gpio_set_level(led_mode_clean, 0);
		break;
	case ENUM_OP_MODE_CLEAN:
		gpio_set_level(led_mode_clean, 1);
		break;
	default:
		device_operation_mode = ENUM_OP_MODE_MANUAL;
		gpio_set_level(led_auto_manual, 1);

		break;
	}
	 */
}


void turnon_motor(time_t on_time, motor_trigger_def trigger, uint16_t pfc_speed, uint16_t rs485_speed)
{
	time_t current_unix_time;
	time(&current_unix_time);

	if(motor_port.on_target == 1)
	{
		motor_port.speed_change = 1;
		ESP_LOGE("--->", "Speed change");
	}
	else
	{
		motor_port.speed_change = 0;
	}

	motor_port.on_target = 1;
	motor_port.force_update = 1;
	motor_port.stop_time = current_unix_time + on_time;
	motor_port.trigger = trigger;
	motor_port.target_speed_pfc = pfc_speed;
	motor_port.target_speed_mbus = rs485_speed;
	motor_port.force_stop = 0;
	if( (priming_data.is_enabled == 1))
	{
		if( motor_port.speed_change == 0)
		{
			motor_port.priming_time_stop = current_unix_time + priming_data.priming_time;
		}
		else
		{
			ESP_LOGE("--->", "Speed change No primming change");
		}
	}
	else
	{
		ESP_LOGE("--->", "Priming stop");
		motor_port.priming_time_stop = 0;
	}
}

void turnoff_motor(motor_trigger_def trigger)
{
	time_t current_unix_time;
	time(&current_unix_time);
	motor_port.on_target = 0;
	motor_port.force_update = 1;
	motor_port.target_speed_pfc = 0;
	motor_port.target_speed_mbus = 0;
	motor_port.priming_time_stop = 0;
	if(motor_port.trigger != ENUM_TRIG_SCHED)
	{
		if(motor_port.keep_mode_active != 1)
			motor_port.stop_time = 0;
		motor_port.force_stop = 0;
	}
	else
	{
		if( (trigger == ENUM_TRIG_SWITCH) || (trigger == ENUM_TRIG_APP))
		{
			motor_port.force_stop = 1;
		}
		else
		{
			motor_port.force_stop = 0;
		}

	}
	motor_port.trigger = trigger;
	motor_port.active_scheduler = 0;
	motor_port.keep_mode_active = 0;

}

void pfc_motor_speed(uint16_t speed)
{
	switch(speed)
	{
	case 1:
	{
		gpio_set_level(contact_pfc_stop, 0);
		gpio_set_level(contact_pfc_n2, 0);
		gpio_set_level(contact_pfc_n3, 0);
		gpio_set_level(contact_pfc_n1, 0);
		vTaskDelay(pdMS_TO_TICKS(500));
		gpio_set_level(contact_pfc_n1, 1); vTaskDelay(pdMS_TO_TICKS(500)); gpio_set_level(contact_pfc_n1, 0);
	}
	break;
	case 2:
	{
		gpio_set_level(contact_pfc_stop, 0);
		gpio_set_level(contact_pfc_n1, 0);
		gpio_set_level(contact_pfc_n3, 0);
		gpio_set_level(contact_pfc_n2, 0);
		vTaskDelay(pdMS_TO_TICKS(500));
		gpio_set_level(contact_pfc_n2, 1); vTaskDelay(pdMS_TO_TICKS(500)); gpio_set_level(contact_pfc_n2, 0);
	}
	break;
	case 3:
	{
		gpio_set_level(contact_pfc_stop, 0);
		gpio_set_level(contact_pfc_n1, 0);
		gpio_set_level(contact_pfc_n2, 0);
		gpio_set_level(contact_pfc_n3, 0);
		vTaskDelay(pdMS_TO_TICKS(500));
		gpio_set_level(contact_pfc_n3, 1); vTaskDelay(pdMS_TO_TICKS(500)); gpio_set_level(contact_pfc_n3, 0);
	}
	break;
	default:
	{
		gpio_set_level(contact_pfc_n1, 0);
		gpio_set_level(contact_pfc_n2, 0);
		gpio_set_level(contact_pfc_n3, 0);
		gpio_set_level(contact_pfc_stop, 0);
		vTaskDelay(pdMS_TO_TICKS(500));
		gpio_set_level(contact_pfc_stop, 1); vTaskDelay(pdMS_TO_TICKS(500)); gpio_set_level(contact_pfc_stop, 0);
	}
	break;
	}
	motor_port.force_update = 0;
}

esp_err_t _motor_off()
{
	if(device_type == ENUM_DEVICE_TYPE_PFC)
	{
		pfc_motor_speed(0);
		return ESP_OK;
	}
	else if(device_type == ENUM_DEVICE_TYPE_RS485)
	{
		if(ESP_OK == modbus_start_stop_motor(0, 0))
		{
			motor_port.force_update = 0;
			return ESP_OK;
		}

	}
	return ESP_FAIL;
}

esp_err_t _motor_on(uint16_t pfc_speed, uint16_t mbus_speed)
{
	uint16_t speed;

	if(device_type == ENUM_DEVICE_TYPE_PFC)
	{
		pfc_motor_speed(pfc_speed);
		return ESP_OK;
	}
	else if(device_type == ENUM_DEVICE_TYPE_RS485)
	{
		if(motor_port.speed_change == 0)
		{
			ESP_LOGE("--->1", "Speed change = 0");
			if(priming_data.is_enabled)
			{
				ESP_LOGE("--->2", "Speed change = 0");
				speed = priming_data.priming_speed;
			}
			else
			{
				ESP_LOGE("--->3", "Speed change = 0");
				speed = motor_port.target_speed_mbus;
			}

			if(ESP_OK == modbus_start_stop_motor(1, speed))
			{
				motor_port.force_update = 0;
				return ESP_OK;
			}
		}
		else
		{
			ESP_LOGE("--->1", "Speed change = 1");
			if(priming_data.is_enabled && (motor_port.priming_time_stop != 0))
			{

			}
			else
			{
				if(ESP_OK == modbus_speed_change_motor(motor_port.target_speed_mbus))
				{
					motor_port.force_update = 0;
					return ESP_OK;
				}
			}
		}
	}
	return ESP_FAIL;
}

uint8_t _check_for_motor(uint8_t index, int *value)
{
	mbus_reg_type reg;

	uint8_t fault = 0;
	uint16_t fault_val = 0;

	if(mbus_map_element[index].enable)
	{
		ESP_LOGI(TAG, "Checking for fault-1");
		if(ESP_OK == modbus_read_holding_reg(mbus_comm_parameter.slave_id, mbus_map_element[index].addr, 1, &reg))
		{
			if(reg.word != mbus_map_element[index].parameter)
			{
				ESP_LOGE("-->", "Error FAULT");
				fault = 1;
				fault_val = reg.word;
			}
			else
			{
				ESP_LOGI(TAG, "No FAULT");
				fault = 0;
				fault_val = 0;
			}
		}
	}
	*value = fault_val;
	return fault;
}

void check_for_motor_err(motor_err_def *err)
{


	err->error_no[0] = _check_for_motor(ENUM_MBUS_REG_FR_1, &err->value[0]);
	err->error_no[1] = _check_for_motor(ENUM_MBUS_REG_FR_2, &err->value[1]);
	err->error_no[2] = _check_for_motor(ENUM_MBUS_REG_FR_3, &err->value[2]);
	err->error_no[3] = _check_for_motor(ENUM_MBUS_REG_FR_4, &err->value[3]);
	err->error_no[4] = _check_for_motor(ENUM_MBUS_REG_FR_5, &err->value[4]);
	err->error_no[5] = _check_for_motor(ENUM_MBUS_REG_FR_6, &err->value[5]);

}

void task_timed_action(void *pv)
{
	time_t current_unix_time;
	time_t soft_time_err_check;

	motor_err_def motor_err;

	xEventGroupWaitBits(common_events_group, 1 << enum_time_synced, false, true, portMAX_DELAY);

	time(&soft_time_err_check);
	soft_time_err_check = soft_time_err_check - 10;
	memset(&motor_err, 0, sizeof(motor_err_def));

	time_t hb_start_time;

	time(&current_unix_time);

	hb_start_time = current_unix_time;

	uint16_t temp;
	while(1)
	{
		time(&current_unix_time);
		if((motor_port.on_actual != motor_port.on_target) || (motor_port.force_update == 1))
		{
			if(motor_port.on_target == 1)
			{
				if(ESP_OK == _motor_on(motor_port.target_speed_pfc, motor_port.target_speed_mbus))
				{
					motor_port.on_actual = 1;
					ESP_LOGE("-->", "Motor is turned on");
					action_get_speed(NULL);
				}
			}
			else
			{
				if(ESP_OK == _motor_off())
				{
					motor_port.on_actual = 0;
					ESP_LOGE("-->", "Motor is turned off");
					action_get_speed(NULL);
				}
			}
		}

		if( (motor_port.stop_time <= current_unix_time))
		{
			motor_port.force_stop = 0;
			motor_port.stop_time = 0;
			motor_port.priming_time_stop = 0;
			if(motor_port.on_target == 1)
			{
				turnoff_motor(ENUM_TRIG_TIMER);
			}
			// Added by harry
			if(motor_port.keep_mode_active){
				motor_port.keep_mode_active = 0;
				action_get_speed(NULL);
			}
		}else if( (motor_port.stop_time - current_unix_time == 5) && (!motor_port.action_port_check)) {
			motor_port.action_port_check = true;
			action_port_check(NULL);
		}

		if(device_type == ENUM_DEVICE_TYPE_RS485)
		{
			if( (motor_port.priming_time_stop <= current_unix_time) )
			{
				if(motor_port.priming_time_stop == 0)
				{

				}
				else
				{
					ESP_LOGI(TAG, "Priming time is over");
					motor_port.priming_time_stop = 0;
					modbus_speed_change_motor(motor_port.target_speed_mbus);
				}
			}
		}

		if((device_type == ENUM_DEVICE_TYPE_RS485) && (xEventGroupGetBits(common_events_group) & (1 << enum_connected_to_mqtt)))
		{
			if((current_unix_time-soft_time_err_check) >= 60)
			{
				ESP_LOGI("--->", "Checking error ");
				soft_time_err_check = current_unix_time;
				check_for_motor_err(&motor_err);

				if( motor_err.error_no[0] | motor_err.error_no[1] | motor_err.error_no[2] |
						motor_err.error_no[3] | motor_err.error_no[4] | motor_err.error_no[5] )
				{
					if( (motor_err.prv_value[0] != motor_err.value[0]) ||(motor_err.prv_value[1] != motor_err.value[1]) ||
							(motor_err.prv_value[2] != motor_err.value[2]) ||(motor_err.prv_value[3] != motor_err.value[3]) ||
							(motor_err.prv_value[4] != motor_err.value[4]) ||(motor_err.prv_value[5] != motor_err.value[5])
					)
					{
						action_chek_motor_fault(NULL);
					}
					for(int i = 0; i < 6; i++)
					{
						motor_err.prv_value[i] = motor_err.value[i];
					}

				}


				if(ESP_OK != modbus_get_actual_speed(&temp))
				{
					ESP_LOGI("--->", "Error in rs485 communication");
					rs_485_comm_err = 1;
				}
				else
				{
					rs_485_comm_err = 0;
				}
			}
		}



		//Timeout for hb
		if(hb_data.enable == 1)
		{
			ESP_LOGI(TAG, "HB time to send = %ld, Interval = %d", current_unix_time - hb_start_time, hb_data.interval);
			if( (current_unix_time - hb_start_time) >= hb_data.interval )
			{
				hb_start_time = current_unix_time;
				send_hb();
			}
		}

		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

void task_sched_start_action(void *pv)
{
	struct tm tm_current_unix_time;
	time_t current_unix_time;

	sched_def scheduler_current;
	xEventGroupWaitBits(common_events_group, 1 << enum_time_synced, false, true, portMAX_DELAY);

	while(1)
	{
		time(&current_unix_time);
		localtime_r(&current_unix_time, &tm_current_unix_time);

		for(int i = 0; i < MAX_SCHED; i++)
		{
			scheduler_read(i+1, &scheduler_current);
			if((scheduler_current.enable == 1) && (scheduler_current.day & (1 << tm_current_unix_time.tm_wday)))
			{
				ESP_LOGI(TAG, "Schedule No:%d Enable:%d HH:%d MM:%d Action=%d Speed_PFC=%d Speed_MBUS=%d",
						i+1, scheduler_current.enable, scheduler_current.start_time_HH, scheduler_current.start_time_MM,
						scheduler_current.start_stop_value, scheduler_current.speed_pfc, scheduler_current.speed_mbus);


				if( (scheduler_current.start_time_HH == tm_current_unix_time.tm_hour) &&
						(scheduler_current.start_time_MM == tm_current_unix_time.tm_min)
				)
				{
					if(scheduler_executed[i] == 0)
					{
						scheduler_executed[i] = 1;
						ESP_LOGI(TAG,"");
						ESP_LOGI(TAG,"Time to execute scheduler: %d", i+1);
						ESP_LOGI(TAG,"");
						if(device_operation_mode == ENUM_OP_MODE_AUTO)
						{
							if(scheduler_current.start_stop_value == 1)
							{
								turnon_motor(MAX_RUN_TIME*60, ENUM_TRIG_APP, scheduler_current.speed_pfc, scheduler_current.speed_mbus);
								motor_port.active_scheduler = i+1;
							}
							else
							{
								turnoff_motor(ENUM_TRIG_SCHED);
							}
						}
						else
						{
							missed_scheduler(i+1);
						}

					}
				}
				else
				{
					scheduler_executed[i] = 0;
				}


			}
		}

		/*
		if( (device_operation_mode == ENUM_OP_MODE_AUTO) && (motor_port.on_target == 0) )
		{

			for(int i = 0; i < MAX_SCHED; i++)
			{
				scheduler_read(i+1, &scheduler_current);
				if((scheduler_current.enable == 1) && (scheduler_current.day & (1 << tm_current_unix_time.tm_wday)))
				{
					memcpy((void *)&tm_scheduler_unix_start_time, (void *)&tm_current_unix_time, sizeof(struct tm));

					tm_scheduler_unix_start_time.tm_hour = scheduler_current.start_time_HH;
					tm_scheduler_unix_start_time.tm_min = scheduler_current.start_time_MM;
					tm_scheduler_unix_start_time.tm_sec = 0;
					scheduler_unix_start_time = mktime(&tm_scheduler_unix_start_time);

					if( (scheduler_unix_start_time <= current_unix_time) &&
							( (scheduler_unix_start_time + (scheduler_current.duration*60)) >= current_unix_time )
					)
					{
						if(motor_port.force_stop == 0)
						{
							turnon_motor((scheduler_unix_start_time + (scheduler_current.duration*60))-current_unix_time, ENUM_TRIG_SCHED, scheduler_current.speed_pfc, scheduler_current.speed_mbus);
						}
						break;
					}
					else
					{

					}
				}
			}
		}
		 */

		vTaskDelay(pdMS_TO_TICKS(5000));
	}
}


void task_indication_leds(void *pv)
{
	led_state_enum led_current_state;
	led_state_enum led_next_state;


	led_current_state = 0;
	led_next_state = 0;

	wifi_led_states_enum wifi_led_states;
	server_led_states_enum server_led_states;
	n1_led_state_enum n1_led_states = -1;
	n2_led_state_enum n2_led_states = -1;
	n3_led_state_enum n3_led_states = -1;
	ns_led_state_enum ns_led_states = -1;
	led_rs485_state_enum rs485_led_states = -1;




	while(1)
	{
		if( (xEventGroupGetBits(common_events_group) & (1 << enum_ap_on)) && (prov_data.isProvisioned != 1) )
		{
			wifi_led_states = ENUM_WIFI_LED_NO_PROV;
		}
		else if(xEventGroupGetBits(common_events_group) & (1 << enum_wifi_connected))
		{
			wifi_led_states = ENUM_WIFI_LED_WIFI_OK;
		}
		else
		{
			wifi_led_states = ENUM_WIFI_LED_WIFI_ERR;
		}

		if( (xEventGroupGetBits(common_events_group) & (1 << enum_ap_on)) && (prov_data.isProvisioned != 1) )
		{
			server_led_states = ENUM_SERVER_LED_NO_PROV;
		}
		else if(xEventGroupGetBits(common_events_group) & (1 << enum_start_ota))
		{
			server_led_states = ENUM_SERVER_IN_OTA;
		}
		else if(xEventGroupGetBits(common_events_group) & (1 << enum_connected_to_mqtt))
		{
			server_led_states = ENUM_SERVER_CONNECTED;
		}
		else
		{
			server_led_states = ENUM_SERVER_NOT_CONNECTED;
		}

		if(device_type == ENUM_DEVICE_TYPE_PFC)
		{
			rs485_led_states = ENUM_RS485_LED_OFF;
			gpio_set_level(led_mode_pfc, 1);
		}
		if(device_type == ENUM_DEVICE_TYPE_RS485)
		{
			gpio_set_level(led_mode_pfc, 0);
			if(rs_485_comm_err == 0)
			{
				rs485_led_states = ENUM_RS485_LED_ON;
			}
			else
			{
				rs485_led_states = ENUM_RS485_LED_FLASH;
			}
		}


		switch(device_operation_mode)
		{
		case ENUM_OP_MODE_MANUAL:
		{
			n1_led_states = ENUM_N1_LED_OFF;
			n2_led_states = ENUM_N2_LED_OFF;
			n3_led_states = ENUM_N3_LED_OFF;
			ns_led_states = ENUM_NS_LED_OFF;

			if(device_type == ENUM_DEVICE_TYPE_PFC)
			{
				switch(motor_port.target_speed_pfc)
				{
				case 1:	n1_led_states = ENUM_N1_LED_ON; break;
				case 2: n2_led_states = ENUM_N2_LED_ON; break;
				case 3:	n3_led_states = ENUM_N3_LED_ON; break;
				case 0:	ns_led_states = ENUM_NS_LED_ON;	break;
				default: break;
				}
			}

			if(device_type == ENUM_DEVICE_TYPE_RS485)
			{
				if(motor_port.target_speed_mbus != 0)
				{
					n1_led_states = ENUM_N1_LED_ON;
					n2_led_states = ENUM_N2_LED_ON;
					n3_led_states = ENUM_N3_LED_ON;
					ns_led_states = ENUM_NS_LED_OFF;
				}
				else
				{
					n1_led_states = ENUM_N1_LED_OFF;
					n2_led_states = ENUM_N2_LED_OFF;
					n3_led_states = ENUM_N3_LED_OFF;
					ns_led_states = ENUM_NS_LED_ON;
				}
			}
			break;
		}
		case ENUM_OP_MODE_AUTO:
		{
			n1_led_states = ENUM_N1_LED_OFF;
			n2_led_states = ENUM_N2_LED_OFF;
			n3_led_states = ENUM_N3_LED_OFF;
			ns_led_states = ENUM_NS_LED_OFF;
			if(motor_port.trigger != ENUM_TRIG_SCHED)
			{
				if(device_type == ENUM_DEVICE_TYPE_PFC)
				{
					switch(motor_port.target_speed_pfc)
					{
					case 1:	n1_led_states = ENUM_N1_LED_ON; break;
					case 2: n2_led_states = ENUM_N2_LED_ON; break;
					case 3:	n3_led_states = ENUM_N3_LED_ON; break;
					case 0:	ns_led_states = ENUM_NS_LED_ON;	break;
					default: break;
					}
				}
				if(device_type == ENUM_DEVICE_TYPE_RS485)
				{
					if(motor_port.target_speed_mbus != 0)
					{
						n1_led_states = ENUM_N1_LED_ON;
						n2_led_states = ENUM_N2_LED_ON;
						n3_led_states = ENUM_N3_LED_ON;
						ns_led_states = ENUM_NS_LED_OFF;
					}
					else
					{
						n1_led_states = ENUM_N1_LED_OFF;
						n2_led_states = ENUM_N2_LED_OFF;
						n3_led_states = ENUM_N3_LED_OFF;
						ns_led_states = ENUM_NS_LED_ON;
					}
				}
			}
			else
			{
				if(device_type == ENUM_DEVICE_TYPE_PFC)
				{
					switch(motor_port.target_speed_pfc)
					{
					case 1:	n1_led_states = ENUM_N1_LED_ON; break;
					case 2: n2_led_states = ENUM_N2_LED_ON; break;
					case 3:	n3_led_states = ENUM_N3_LED_ON; break;
					case 0:	ns_led_states = ENUM_NS_LED_ON;	break;
					default: break;
					}
				}
				if(device_type == ENUM_DEVICE_TYPE_RS485)
				{
					if(motor_port.target_speed_mbus != 0)
					{
						n1_led_states = ENUM_N1_LED_ON;
						n2_led_states = ENUM_N2_LED_ON;
						n3_led_states = ENUM_N3_LED_ON;
						ns_led_states = ENUM_NS_LED_OFF;
					}
					else
					{
						n1_led_states = ENUM_N1_LED_OFF;
						n2_led_states = ENUM_N2_LED_OFF;
						n3_led_states = ENUM_N3_LED_OFF;
						ns_led_states = ENUM_NS_LED_ON;
					}
				}

			}
			break;
		}
		case ENUM_OP_MODE_CLEAN:
		{
			n1_led_states = ENUM_N1_LED_OFF;
			n2_led_states = ENUM_N2_LED_OFF;
			n3_led_states = ENUM_N3_LED_OFF;
			ns_led_states = ENUM_NS_LED_ON;
			break;
		}
		default:
		{
			break;
		}
		}




		//Auto manual
		if(device_operation_mode == ENUM_OP_MODE_AUTO)
		{
			gpio_set_level(led_auto_manual, 0);
			gpio_set_level(led_mode_clean, 0);
		}
		if(device_operation_mode == ENUM_OP_MODE_MANUAL)
		{
			gpio_set_level(led_auto_manual, 1);
			gpio_set_level(led_mode_clean, 0);
		}
		if(device_operation_mode == ENUM_OP_MODE_CLEAN)
		{
			//gpio_set_level(led_auto_manual, (1 == gpio_get_level(led_auto_manual))?0:1);
			gpio_set_level(led_mode_clean, 1);
		}
		// NS LED
		switch(ns_led_states)
		{
		case ENUM_NS_LED_OFF: gpio_set_level(led_pfc_stop, 0); break;
		case ENUM_NS_LED_ON: gpio_set_level(led_pfc_stop, 1); break;
		default: break;
		}

		led_current_state = led_next_state;
		switch(led_current_state)
		{
		case ENUM_LED_STATE_1:
		{
			//rs485 led
			switch(rs485_led_states)
			{
			case ENUM_RS485_LED_OFF: gpio_set_level(led_mode_rs485, 0); break;
			case ENUM_RS485_LED_ON: gpio_set_level(led_mode_rs485, 1); break;
			case ENUM_RS485_LED_FLASH: gpio_set_level(led_mode_rs485, 1); break;
			default: gpio_set_level(led_mode_rs485, 0); break;
			}

			//Wifi led
			switch(wifi_led_states)
			{
			case ENUM_WIFI_LED_NO_PROV:
				gpio_set_level(led_wifi, 1);
				break;
			case ENUM_WIFI_LED_WIFI_ERR:
				gpio_set_level(led_wifi, 1);
				break;
			case ENUM_WIFI_LED_WIFI_OK:
				gpio_set_level(led_wifi, 1);
				break;
			default:
				gpio_set_level(led_wifi, 0);
				break;
			}

			//Server led
			switch(server_led_states)
			{
			case ENUM_SERVER_LED_NO_PROV:
				gpio_set_level(led_server, 1);
				break;
			case ENUM_SERVER_NOT_CONNECTED:
				gpio_set_level(led_server, 1);
				break;
			case ENUM_SERVER_IN_OTA:
				gpio_set_level(led_server, 1);
				break;
			case ENUM_SERVER_CONNECTED:
				gpio_set_level(led_server, 1);
				break;
			default:
				gpio_set_level(led_server, 0);
				break;
			}

			// N1 LED
			switch(n1_led_states)
			{
			case ENUM_N1_LED_OFF: gpio_set_level(led_pfc_n1, 0); break;
			case ENUM_N1_LED_ON: gpio_set_level(led_pfc_n1, 1); break;
			case ENUM_N1_LED_FLASH: gpio_set_level(led_pfc_n1, 1); break;
			case ENUM_N1_LED_SCHED: gpio_set_level(led_pfc_n1, 1); break;
			default: break;
			}

			// N2 LED
			switch(n2_led_states)
			{
			case ENUM_N2_LED_OFF: gpio_set_level(led_pfc_n2, 0); break;
			case ENUM_N2_LED_ON: gpio_set_level(led_pfc_n2, 1); break;
			case ENUM_N2_LED_FLASH: gpio_set_level(led_pfc_n2, 1); break;
			case ENUM_N2_LED_SCHED: gpio_set_level(led_pfc_n2, 1); break;
			default: break;
			}

			// N3 LED
			switch(n3_led_states)
			{
			case ENUM_N3_LED_OFF: gpio_set_level(led_pfc_n3, 0); break;
			case ENUM_N3_LED_ON: gpio_set_level(led_pfc_n3, 1); break;
			case ENUM_N3_LED_FLASH: gpio_set_level(led_pfc_n3, 1); break;
			case ENUM_N3_LED_SCHED: gpio_set_level(led_pfc_n3, 1); break;
			default: break;
			}



			led_next_state = ENUM_LED_STATE_2;
			break;
		}



		case ENUM_LED_STATE_2:
		{
			//rs485 led
			switch(rs485_led_states)
			{
			case ENUM_RS485_LED_OFF: gpio_set_level(led_mode_rs485, 0); break;
			case ENUM_RS485_LED_ON: gpio_set_level(led_mode_rs485, 1); break;
			case ENUM_RS485_LED_FLASH: gpio_set_level(led_mode_rs485, 0); break;
			default: gpio_set_level(led_mode_rs485, 0); break;
			}

			//Wifi led
			switch(wifi_led_states)
			{
			case ENUM_WIFI_LED_NO_PROV:
				gpio_set_level(led_wifi, 0);
				break;
			case ENUM_WIFI_LED_WIFI_ERR:
				gpio_set_level(led_wifi, 0);
				break;
			case ENUM_WIFI_LED_WIFI_OK:
				gpio_set_level(led_wifi, 1);
				break;
			default:
				gpio_set_level(led_wifi, 0);
				break;
			}

			//Server led
			switch(server_led_states)
			{
			case ENUM_SERVER_LED_NO_PROV:
				gpio_set_level(led_server, 0);
				break;
			case ENUM_SERVER_NOT_CONNECTED:
				gpio_set_level(led_server, 0);
				break;
			case ENUM_SERVER_IN_OTA:
				gpio_set_level(led_server, 0);
				break;
			case ENUM_SERVER_CONNECTED:
				gpio_set_level(led_server, 1);
				break;
			default:
				gpio_set_level(led_server, 0);
				break;
			}

			// N1 LED
			switch(n1_led_states)
			{
			case ENUM_N1_LED_OFF: gpio_set_level(led_pfc_n1, 0); break;
			case ENUM_N1_LED_ON: gpio_set_level(led_pfc_n1, 1); break;
			case ENUM_N1_LED_FLASH: gpio_set_level(led_pfc_n1, 0); break;
			case ENUM_N1_LED_SCHED: gpio_set_level(led_pfc_n1, 0); break;
			default: break;
			}

			// N2 LED
			switch(n2_led_states)
			{
			case ENUM_N2_LED_OFF: gpio_set_level(led_pfc_n2, 0); break;
			case ENUM_N2_LED_ON: gpio_set_level(led_pfc_n2, 1); break;
			case ENUM_N2_LED_FLASH: gpio_set_level(led_pfc_n2, 0); break;
			case ENUM_N2_LED_SCHED: gpio_set_level(led_pfc_n2, 0); break;
			default: break;
			}

			// N3 LED
			switch(n3_led_states)
			{
			case ENUM_N3_LED_OFF: gpio_set_level(led_pfc_n3, 0); break;
			case ENUM_N3_LED_ON: gpio_set_level(led_pfc_n3, 1); break;
			case ENUM_N3_LED_FLASH: gpio_set_level(led_pfc_n3, 0); break;
			case ENUM_N3_LED_SCHED: gpio_set_level(led_pfc_n3, 0); break;
			default: break;
			}


			led_next_state = ENUM_LED_STATE_3;
			break;
		}
		case ENUM_LED_STATE_3:
		{
			//rs485 led
			switch(rs485_led_states)
			{
			case ENUM_RS485_LED_OFF: gpio_set_level(led_mode_rs485, 0); break;
			case ENUM_RS485_LED_ON: gpio_set_level(led_mode_rs485, 1); break;
			case ENUM_RS485_LED_FLASH: gpio_set_level(led_mode_rs485, 0); break;
			default: gpio_set_level(led_mode_rs485, 0); break;
			}

			//Wifi led
			switch(wifi_led_states)
			{
			case ENUM_WIFI_LED_NO_PROV:
				gpio_set_level(led_wifi, 0);
				break;
			case ENUM_WIFI_LED_WIFI_ERR:
				gpio_set_level(led_wifi, 1);
				break;
			case ENUM_WIFI_LED_WIFI_OK:
				gpio_set_level(led_wifi, 1);
				break;
			default:
				gpio_set_level(led_wifi, 0);
				break;
			}

			//Server led
			switch(server_led_states)
			{
			case ENUM_SERVER_LED_NO_PROV:
				gpio_set_level(led_server, 0);
				break;
			case ENUM_SERVER_NOT_CONNECTED:
				gpio_set_level(led_server, 1);
				break;
			case ENUM_SERVER_IN_OTA:
				gpio_set_level(led_server, 1);
				break;
			case ENUM_SERVER_CONNECTED:
				gpio_set_level(led_server, 1);
				break;
			default:
				gpio_set_level(led_server, 0);
				break;
			}

			// N1 LED
			switch(n1_led_states)
			{
			case ENUM_N1_LED_OFF: gpio_set_level(led_pfc_n1, 0); break;
			case ENUM_N1_LED_ON: gpio_set_level(led_pfc_n1, 1); break;
			case ENUM_N1_LED_FLASH: gpio_set_level(led_pfc_n1, 0); break;
			case ENUM_N1_LED_SCHED: gpio_set_level(led_pfc_n1, 1); break;
			default: break;
			}

			// N2 LED
			switch(n2_led_states)
			{
			case ENUM_N2_LED_OFF: gpio_set_level(led_pfc_n2, 0); break;
			case ENUM_N2_LED_ON: gpio_set_level(led_pfc_n2, 1); break;
			case ENUM_N2_LED_FLASH: gpio_set_level(led_pfc_n2, 0); break;
			case ENUM_N2_LED_SCHED: gpio_set_level(led_pfc_n2, 1); break;
			default: break;
			}

			// N3 LED
			switch(n3_led_states)
			{
			case ENUM_N3_LED_OFF: gpio_set_level(led_pfc_n3, 0); break;
			case ENUM_N3_LED_ON: gpio_set_level(led_pfc_n3, 1); break;
			case ENUM_N3_LED_FLASH: gpio_set_level(led_pfc_n3, 0); break;
			case ENUM_N3_LED_SCHED: gpio_set_level(led_pfc_n3, 1); break;
			default: break;
			}
			led_next_state = ENUM_LED_STATE_4;
			break;
		}
		case ENUM_LED_STATE_4:
		{
			//rs485 led
			switch(rs485_led_states)
			{
			case ENUM_RS485_LED_OFF: gpio_set_level(led_mode_rs485, 0); break;
			case ENUM_RS485_LED_ON: gpio_set_level(led_mode_rs485, 1); break;
			case ENUM_RS485_LED_FLASH: gpio_set_level(led_mode_rs485, 0); break;
			default: gpio_set_level(led_mode_rs485, 0); break;
			}

			//Wifi led
			switch(wifi_led_states)
			{
			case ENUM_WIFI_LED_NO_PROV:
				gpio_set_level(led_wifi, 0);
				break;
			case ENUM_WIFI_LED_WIFI_ERR:
				gpio_set_level(led_wifi, 0);
				break;
			case ENUM_WIFI_LED_WIFI_OK:
				gpio_set_level(led_wifi, 1);
				break;
			default:
				gpio_set_level(led_wifi, 0);
				break;
			}

			//Server led
			switch(server_led_states)
			{
			case ENUM_SERVER_LED_NO_PROV:
				gpio_set_level(led_server, 0);
				break;
			case ENUM_SERVER_NOT_CONNECTED:
				gpio_set_level(led_server, 0);
				break;
			case ENUM_SERVER_IN_OTA:
				gpio_set_level(led_server, 0);
				break;
			case ENUM_SERVER_CONNECTED:
				gpio_set_level(led_server, 1);
				break;
			default:
				gpio_set_level(led_server, 0);
				break;
			}

			// N1 LED
			switch(n1_led_states)
			{
			case ENUM_N1_LED_OFF: gpio_set_level(led_pfc_n1, 0); break;
			case ENUM_N1_LED_ON: gpio_set_level(led_pfc_n1, 1); break;
			case ENUM_N1_LED_FLASH: gpio_set_level(led_pfc_n1, 0); break;
			case ENUM_N1_LED_SCHED: gpio_set_level(led_pfc_n1, 0); break;
			default: break;
			}

			// N2 LED
			switch(n2_led_states)
			{
			case ENUM_N2_LED_OFF: gpio_set_level(led_pfc_n2, 0); break;
			case ENUM_N2_LED_ON: gpio_set_level(led_pfc_n2, 1); break;
			case ENUM_N2_LED_FLASH: gpio_set_level(led_pfc_n2, 0); break;
			case ENUM_N2_LED_SCHED: gpio_set_level(led_pfc_n2, 0); break;
			default: break;
			}

			// N3 LED
			switch(n3_led_states)
			{
			case ENUM_N3_LED_OFF: gpio_set_level(led_pfc_n3, 0); break;
			case ENUM_N3_LED_ON: gpio_set_level(led_pfc_n3, 1); break;
			case ENUM_N3_LED_FLASH: gpio_set_level(led_pfc_n3, 0); break;
			case ENUM_N3_LED_SCHED: gpio_set_level(led_pfc_n3, 0); break;
			default: break;
			}

			led_next_state = ENUM_LED_STATE_5;
			break;
		}
		case ENUM_LED_STATE_5:
		{
			//rs485 led
			switch(rs485_led_states)
			{
			case ENUM_RS485_LED_OFF: gpio_set_level(led_mode_rs485, 0); break;
			case ENUM_RS485_LED_ON: gpio_set_level(led_mode_rs485, 1); break;
			case ENUM_RS485_LED_FLASH: gpio_set_level(led_mode_rs485, 0); break;
			default: gpio_set_level(led_mode_rs485, 0); break;
			}

			//Wifi led
			switch(wifi_led_states)
			{
			case ENUM_WIFI_LED_NO_PROV:
				gpio_set_level(led_wifi, 0);
				break;
			case ENUM_WIFI_LED_WIFI_ERR:
				gpio_set_level(led_wifi, 0);
				break;
			case ENUM_WIFI_LED_WIFI_OK:
				gpio_set_level(led_wifi, 1);
				break;
			default:
				gpio_set_level(led_wifi, 0);
				break;
			}

			//Server led
			switch(server_led_states)
			{
			case ENUM_SERVER_LED_NO_PROV:
				gpio_set_level(led_server, 0);
				break;
			case ENUM_SERVER_NOT_CONNECTED:
				gpio_set_level(led_server, 0);
				break;
			case ENUM_SERVER_IN_OTA:
				gpio_set_level(led_server, 1);
				break;
			case ENUM_SERVER_CONNECTED:
				gpio_set_level(led_server, 1);
				break;
			default:
				gpio_set_level(led_server, 0);
				break;
			}

			// N1 LED
			switch(n1_led_states)
			{
			case ENUM_N1_LED_OFF: gpio_set_level(led_pfc_n1, 0); break;
			case ENUM_N1_LED_ON: gpio_set_level(led_pfc_n1, 1); break;
			case ENUM_N1_LED_FLASH: gpio_set_level(led_pfc_n1, 0); break;
			case ENUM_N1_LED_SCHED: gpio_set_level(led_pfc_n1, 0); break;
			default: break;
			}

			// N2 LED
			switch(n2_led_states)
			{
			case ENUM_N2_LED_OFF: gpio_set_level(led_pfc_n2, 0); break;
			case ENUM_N2_LED_ON: gpio_set_level(led_pfc_n2, 1); break;
			case ENUM_N2_LED_FLASH: gpio_set_level(led_pfc_n2, 0); break;
			case ENUM_N2_LED_SCHED: gpio_set_level(led_pfc_n2, 0); break;
			default: break;
			}

			// N3 LED
			switch(n3_led_states)
			{
			case ENUM_N3_LED_OFF: gpio_set_level(led_pfc_n3, 0); break;
			case ENUM_N3_LED_ON: gpio_set_level(led_pfc_n3, 1); break;
			case ENUM_N3_LED_FLASH: gpio_set_level(led_pfc_n3, 0); break;
			case ENUM_N3_LED_SCHED: gpio_set_level(led_pfc_n3, 0); break;
			default: break;
			}

			led_next_state = ENUM_LED_STATE_6;
			break;
		}
		case ENUM_LED_STATE_6:
		{
			//rs485 led
			switch(rs485_led_states)
			{
			case ENUM_RS485_LED_OFF: gpio_set_level(led_mode_rs485, 0); break;
			case ENUM_RS485_LED_ON: gpio_set_level(led_mode_rs485, 1); break;
			case ENUM_RS485_LED_FLASH: gpio_set_level(led_mode_rs485, 0); break;
			default: gpio_set_level(led_mode_rs485, 0); break;
			}


			//Wifi led
			switch(wifi_led_states)
			{
			case ENUM_WIFI_LED_NO_PROV:
				gpio_set_level(led_wifi, 0);
				break;
			case ENUM_WIFI_LED_WIFI_ERR:
				gpio_set_level(led_wifi, 0);
				break;
			case ENUM_WIFI_LED_WIFI_OK:
				gpio_set_level(led_wifi, 1);
				break;
			default:
				gpio_set_level(led_wifi, 0);
				break;
			}

			//Server led
			switch(server_led_states)
			{
			case ENUM_SERVER_LED_NO_PROV:
				gpio_set_level(led_server, 0);
				break;
			case ENUM_SERVER_NOT_CONNECTED:
				gpio_set_level(led_server, 0);
				break;
			case ENUM_SERVER_IN_OTA:
				gpio_set_level(led_server, 0);
				break;
			case ENUM_SERVER_CONNECTED:
				gpio_set_level(led_server, 1);
				break;
			default:
				gpio_set_level(led_server, 0);
				break;
			}

			// N1 LED
			switch(n1_led_states)
			{
			case ENUM_N1_LED_OFF: gpio_set_level(led_pfc_n1, 0); break;
			case ENUM_N1_LED_ON: gpio_set_level(led_pfc_n1, 1); break;
			case ENUM_N1_LED_FLASH: gpio_set_level(led_pfc_n1, 0); break;
			case ENUM_N1_LED_SCHED: gpio_set_level(led_pfc_n1, 0); break;
			default: break;
			}

			// N2 LED
			switch(n2_led_states)
			{
			case ENUM_N2_LED_OFF: gpio_set_level(led_pfc_n2, 0); break;
			case ENUM_N2_LED_ON: gpio_set_level(led_pfc_n2, 1); break;
			case ENUM_N2_LED_FLASH: gpio_set_level(led_pfc_n2, 0); break;
			case ENUM_N2_LED_SCHED: gpio_set_level(led_pfc_n2, 0); break;
			default: break;
			}

			// N3 LED
			switch(n3_led_states)
			{
			case ENUM_N3_LED_OFF: gpio_set_level(led_pfc_n3, 0); break;
			case ENUM_N3_LED_ON: gpio_set_level(led_pfc_n3, 1); break;
			case ENUM_N3_LED_FLASH: gpio_set_level(led_pfc_n3, 0); break;
			case ENUM_N3_LED_SCHED: gpio_set_level(led_pfc_n3, 0); break;
			default: break;
			}


			led_next_state = ENUM_LED_STATE_7;
			break;
		}
		case ENUM_LED_STATE_7:
		{
			//rs485 led
			switch(rs485_led_states)
			{
			case ENUM_RS485_LED_OFF: gpio_set_level(led_mode_rs485, 0); break;
			case ENUM_RS485_LED_ON: gpio_set_level(led_mode_rs485, 1); break;
			case ENUM_RS485_LED_FLASH: gpio_set_level(led_mode_rs485, 0); break;
			default: gpio_set_level(led_mode_rs485, 0); break;
			}


			//Wifi led
			switch(wifi_led_states)
			{
			case ENUM_WIFI_LED_NO_PROV: gpio_set_level(led_wifi, 0); break;
			case ENUM_WIFI_LED_WIFI_ERR: gpio_set_level(led_wifi, 0); break;
			case ENUM_WIFI_LED_WIFI_OK: gpio_set_level(led_wifi, 1); break;
			default: gpio_set_level(led_wifi, 0); break;
			}

			//Server led
			switch(server_led_states)
			{
			case ENUM_SERVER_LED_NO_PROV: gpio_set_level(led_server, 0); break;
			case ENUM_SERVER_NOT_CONNECTED: gpio_set_level(led_server, 0); break;
			case ENUM_SERVER_IN_OTA:gpio_set_level(led_server, 0);break;
			case ENUM_SERVER_CONNECTED: gpio_set_level(led_server, 1);break;
			default: gpio_set_level(led_server, 0);	break;
			}

			// N1 LED
			switch(n1_led_states)
			{
			case ENUM_N1_LED_OFF: gpio_set_level(led_pfc_n1, 0); break;
			case ENUM_N1_LED_ON: gpio_set_level(led_pfc_n1, 1); break;
			case ENUM_N1_LED_FLASH: gpio_set_level(led_pfc_n1, 0); break;
			case ENUM_N1_LED_SCHED: gpio_set_level(led_pfc_n1, 0); break;
			default: break;
			}

			// N2 LED
			switch(n2_led_states)
			{
			case ENUM_N2_LED_OFF: gpio_set_level(led_pfc_n2, 0); break;
			case ENUM_N2_LED_ON: gpio_set_level(led_pfc_n2, 1); break;
			case ENUM_N2_LED_FLASH: gpio_set_level(led_pfc_n2, 0); break;
			case ENUM_N2_LED_SCHED: gpio_set_level(led_pfc_n2, 0); break;
			default: break;
			}

			// N3 LED
			switch(n3_led_states)
			{
			case ENUM_N3_LED_OFF: gpio_set_level(led_pfc_n3, 0); break;
			case ENUM_N3_LED_ON: gpio_set_level(led_pfc_n3, 1); break;
			case ENUM_N3_LED_FLASH: gpio_set_level(led_pfc_n3, 0); break;
			case ENUM_N3_LED_SCHED: gpio_set_level(led_pfc_n3, 0); break;
			default: break;
			}

			led_next_state = ENUM_LED_STATE_8;
			break;
		}
		case ENUM_LED_STATE_8:
		{
			//rs485 led
			switch(rs485_led_states)
			{
			case ENUM_RS485_LED_OFF: gpio_set_level(led_mode_rs485, 0); break;
			case ENUM_RS485_LED_ON: gpio_set_level(led_mode_rs485, 1); break;
			case ENUM_RS485_LED_FLASH: gpio_set_level(led_mode_rs485, 0); break;
			default: gpio_set_level(led_mode_rs485, 0); break;
			}

			//Wifi led
			switch(wifi_led_states)
			{
			case ENUM_WIFI_LED_NO_PROV: gpio_set_level(led_wifi, 0); break;
			case ENUM_WIFI_LED_WIFI_ERR: gpio_set_level(led_wifi, 0); break;
			case ENUM_WIFI_LED_WIFI_OK:	gpio_set_level(led_wifi, 1); break;
			default:gpio_set_level(led_wifi, 0); break;
			}

			//Server led
			switch(server_led_states)
			{
			case ENUM_SERVER_LED_NO_PROV:
				gpio_set_level(led_server, 0);
				break;
			case ENUM_SERVER_NOT_CONNECTED:
				gpio_set_level(led_server, 0);
				break;
			case ENUM_SERVER_IN_OTA:
				gpio_set_level(led_server, 0);
				break;
			case ENUM_SERVER_CONNECTED:
				gpio_set_level(led_server, 1);
				break;
			default:
				gpio_set_level(led_server, 0);
				break;
			}

			// N1 LED
			switch(n1_led_states)
			{
			case ENUM_N1_LED_OFF: gpio_set_level(led_pfc_n1, 0); break;
			case ENUM_N1_LED_ON: gpio_set_level(led_pfc_n1, 1); break;
			case ENUM_N1_LED_FLASH: gpio_set_level(led_pfc_n1, 0); break;
			case ENUM_N1_LED_SCHED: gpio_set_level(led_pfc_n1, 0); break;
			default: break;
			}

			// N2 LED
			switch(n2_led_states)
			{
			case ENUM_N2_LED_OFF: gpio_set_level(led_pfc_n2, 0); break;
			case ENUM_N2_LED_ON: gpio_set_level(led_pfc_n2, 1); break;
			case ENUM_N2_LED_FLASH: gpio_set_level(led_pfc_n2, 0); break;
			case ENUM_N2_LED_SCHED: gpio_set_level(led_pfc_n2, 0); break;
			default: break;
			}

			// N3 LED
			switch(n3_led_states)
			{
			case ENUM_N3_LED_OFF: gpio_set_level(led_pfc_n3, 0); break;
			case ENUM_N3_LED_ON: gpio_set_level(led_pfc_n3, 1); break;
			case ENUM_N3_LED_FLASH: gpio_set_level(led_pfc_n3, 0); break;
			case ENUM_N3_LED_SCHED: gpio_set_level(led_pfc_n3, 0); break;
			default: break;
			}

			led_next_state = ENUM_LED_STATE_9;
			break;
		}
		case ENUM_LED_STATE_9:
		{
			//rs485 led
			switch(rs485_led_states)
			{
			case ENUM_RS485_LED_OFF: gpio_set_level(led_mode_rs485, 0); break;
			case ENUM_RS485_LED_ON: gpio_set_level(led_mode_rs485, 1); break;
			case ENUM_RS485_LED_FLASH: gpio_set_level(led_mode_rs485, 0); break;
			default: gpio_set_level(led_mode_rs485, 0); break;
			}


			//Wifi led
			switch(wifi_led_states)
			{
			case ENUM_WIFI_LED_NO_PROV: gpio_set_level(led_wifi, 0); break;
			case ENUM_WIFI_LED_WIFI_ERR: gpio_set_level(led_wifi, 0); break;
			case ENUM_WIFI_LED_WIFI_OK: gpio_set_level(led_wifi, 1); break;
			default: gpio_set_level(led_wifi, 0); break;
			}

			//Server led
			switch(server_led_states)
			{
			case ENUM_SERVER_LED_NO_PROV: gpio_set_level(led_server, 0); break;
			case ENUM_SERVER_NOT_CONNECTED: gpio_set_level(led_server, 0); break;
			case ENUM_SERVER_IN_OTA: gpio_set_level(led_server, 0); break;
			case ENUM_SERVER_CONNECTED: gpio_set_level(led_server, 1); break;
			default: gpio_set_level(led_server, 0); break;
			}

			// N1 LED
			switch(n1_led_states)
			{
			case ENUM_N1_LED_OFF: gpio_set_level(led_pfc_n1, 0); break;
			case ENUM_N1_LED_ON: gpio_set_level(led_pfc_n1, 1); break;
			case ENUM_N1_LED_FLASH: gpio_set_level(led_pfc_n1, 0); break;
			case ENUM_N1_LED_SCHED: gpio_set_level(led_pfc_n1, 0); break;
			default: break;
			}

			// N2 LED
			switch(n2_led_states)
			{
			case ENUM_N2_LED_OFF: gpio_set_level(led_pfc_n2, 0); break;
			case ENUM_N2_LED_ON: gpio_set_level(led_pfc_n2, 1); break;
			case ENUM_N2_LED_FLASH: gpio_set_level(led_pfc_n2, 0); break;
			case ENUM_N2_LED_SCHED: gpio_set_level(led_pfc_n2, 0); break;
			default: break;
			}

			// N3 LED
			switch(n3_led_states)
			{
			case ENUM_N3_LED_OFF: gpio_set_level(led_pfc_n3, 0); break;
			case ENUM_N3_LED_ON: gpio_set_level(led_pfc_n3, 1); break;
			case ENUM_N3_LED_FLASH: gpio_set_level(led_pfc_n3, 0); break;
			case ENUM_N3_LED_SCHED: gpio_set_level(led_pfc_n3, 0); break;
			default: break;
			}

			led_next_state = ENUM_LED_STATE_10;
			break;
		}
		case ENUM_LED_STATE_10:
		{
			//rs485 led
			switch(rs485_led_states)
			{
			case ENUM_RS485_LED_OFF: gpio_set_level(led_mode_rs485, 0); break;
			case ENUM_RS485_LED_ON: gpio_set_level(led_mode_rs485, 1); break;
			case ENUM_RS485_LED_FLASH: gpio_set_level(led_mode_rs485, 0); break;
			default: gpio_set_level(led_mode_rs485, 0); break;
			}

			//Wifi led
			switch(wifi_led_states)
			{
			case ENUM_WIFI_LED_NO_PROV: gpio_set_level(led_wifi, 0); break;
			case ENUM_WIFI_LED_WIFI_ERR: gpio_set_level(led_wifi, 0); break;
			case ENUM_WIFI_LED_WIFI_OK: gpio_set_level(led_wifi, 1); break;
			default: gpio_set_level(led_wifi, 0); break;
			}

			//Server led
			switch(server_led_states)
			{
			case ENUM_SERVER_LED_NO_PROV: gpio_set_level(led_server, 0); break;
			case ENUM_SERVER_NOT_CONNECTED: gpio_set_level(led_server, 0); break;
			case ENUM_SERVER_IN_OTA: gpio_set_level(led_server, 0); break;
			case ENUM_SERVER_CONNECTED: gpio_set_level(led_server, 1); break;
			default: gpio_set_level(led_server, 0); break;
			}

			// N1 LED
			switch(n1_led_states)
			{
			case ENUM_N1_LED_OFF: gpio_set_level(led_pfc_n1, 0); break;
			case ENUM_N1_LED_ON: gpio_set_level(led_pfc_n1, 1); break;
			case ENUM_N1_LED_FLASH: gpio_set_level(led_pfc_n1, 0); break;
			case ENUM_N1_LED_SCHED: gpio_set_level(led_pfc_n1, 0); break;
			default: break;
			}

			// N2 LED
			switch(n2_led_states)
			{
			case ENUM_N2_LED_OFF: gpio_set_level(led_pfc_n2, 0); break;
			case ENUM_N2_LED_ON: gpio_set_level(led_pfc_n2, 1); break;
			case ENUM_N2_LED_FLASH: gpio_set_level(led_pfc_n2, 0); break;
			case ENUM_N2_LED_SCHED: gpio_set_level(led_pfc_n2, 0); break;
			default: break;
			}

			// N3 LED
			switch(n3_led_states)
			{
			case ENUM_N3_LED_OFF: gpio_set_level(led_pfc_n3, 0); break;
			case ENUM_N3_LED_ON: gpio_set_level(led_pfc_n3, 1); break;
			case ENUM_N3_LED_FLASH: gpio_set_level(led_pfc_n3, 0); break;
			case ENUM_N3_LED_SCHED: gpio_set_level(led_pfc_n3, 0); break;
			default: break;
			}

			led_next_state = ENUM_LED_STATE_1;
			break;
		}
		default:
			break;
		}

		vTaskDelay(pdMS_TO_TICKS(100));
	}
}
