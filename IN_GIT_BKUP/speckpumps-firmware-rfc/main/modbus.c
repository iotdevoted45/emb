/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#include "modbus.h"

#define MB_PORT_NUM			(2)
#define MB_BAUD_RATE 		(9600)
#define MB_TX_PIN			(17)
#define MB_RX_PIN			(16)
#define MB_RTS_PIN			(19)



esp_err_t modbus_master_init()
{
	mb_communication_info_t comm = {
			.port = MB_PORT_NUM,
			.mode = MB_MODE_RTU,
			.baudrate = mbus_comm_parameter.baud_rate,
			.parity = MB_PARITY_NONE
	};

	void* master_handler = NULL;
	esp_err_t err = mbc_master_init(MB_PORT_SERIAL_MASTER, &master_handler);
	if(master_handler == NULL)
	{
		ESP_LOGI(TAG, "Failed to init modbus Reason: NULL");
		return ESP_FAIL;
	}

	if(ESP_OK != err)
	{
		ESP_LOGI(TAG, "Failed to init modbus: Reason = %d", err);
		return ESP_FAIL;
	}

	err = mbc_master_setup((void*)&comm);
	if(ESP_OK != err)
	{
		ESP_LOGI(TAG, "Error in setting up modbus master, Reason = %d", err);
		return ESP_FAIL;
	}

	err = uart_set_pin(MB_PORT_NUM, MB_TX_PIN, MB_RX_PIN, MB_RTS_PIN, UART_PIN_NO_CHANGE);
	err = mbc_master_start();
	if(ESP_OK != err)
	{
		ESP_LOGI(TAG, "Error in setting modbus master start, Reason = %d", err);
		return ESP_FAIL;
	}

	err = uart_set_mode(MB_PORT_NUM, UART_MODE_RS485_HALF_DUPLEX);
	if(ESP_OK != err)
	{
		ESP_LOGI(TAG, "Error in setting up UART mode, Reason = %d", err);
		return ESP_FAIL;
	}

	vTaskDelay(pdMS_TO_TICKS(1000));

	return ESP_OK;

}

esp_err_t modbus_read_holding_reg(uint8_t slave_addr, uint16_t start_addr, uint8_t reg_count, mbus_reg_type *reg)
{
	esp_err_t err;
	mb_param_request_t mbus_request;

	mbus_request.slave_addr = slave_addr;
	mbus_request.reg_size = reg_count;
	mbus_request.reg_start = start_addr-1;
	mbus_request.command = 3;  //Reading holding register

	xSemaphoreTake(mutex_mbus, portMAX_DELAY);
	err = mbc_master_send_request(&mbus_request, (void *)reg);
	vTaskDelay(pdMS_TO_TICKS(1000));
	xSemaphoreGive(mutex_mbus);
	if(ESP_OK != err)
	{
		return err;
	}
	else
	{
		for(int i = 0; i <reg_count; i++)
		{
			ESP_LOGE("--->", "%2d:Reg = %d, HB = %d, LB = %d", i, reg[i].word, reg[i].byte[1], reg[i].byte[0]);
		}
	}
	return ESP_OK;
}

void task_modbus_scan(void *pV)
{

	while(1)
	{
		xSemaphoreTake(mutex_mbus, portMAX_DELAY);
		xSemaphoreGive(mutex_mbus);
		vTaskDelay(pdMS_TO_TICKS(1000));
	}

}

esp_err_t modbus_write_holding_reg(uint8_t slave_addr, uint16_t addr, mbus_reg_type reg)
{
	esp_err_t err;
	mb_param_request_t mbus_request;



	mbus_request.slave_addr = slave_addr;
	mbus_request.reg_size = 1;
	mbus_request.reg_start = addr-1;
	mbus_request.command = 6;  //Write single holding register


	ESP_LOGE(TAG, "Writing Slave with address %d value = %d", slave_addr, reg.word);
	xSemaphoreTake(mutex_mbus, portMAX_DELAY);


	err = mbc_master_send_request(&mbus_request, (void *)&reg);
	vTaskDelay(pdMS_TO_TICKS(1000));
	xSemaphoreGive(mutex_mbus);
	if(ESP_OK != err)
	{
		return err;
	}

	return ESP_OK;

}

esp_err_t modbus_speed_change_motor(uint16_t new_speed)
{
	esp_err_t err;
	mbus_reg_type reg;

	reg.word = new_speed;
	err = modbus_write_holding_reg(mbus_comm_parameter.slave_id, mbus_map_element[ENUM_MBUS_REG_TAR_SPEED].addr, reg);
	if(ESP_OK != err)
	{
		ESP_LOGI(TAG, "Error in changing speed of motor");
	}

	return err;

}
esp_err_t modbus_start_stop_motor(uint8_t value, uint16_t speed)
{
	esp_err_t err;
	mbus_reg_type reg;
	uint8_t step1 = 0;
	uint8_t step2 = 0;
	uint8_t valid = 0;

	ESP_LOGE(TAG, "Motor Start stop");


	reg.word = speed;
	err = modbus_write_holding_reg(mbus_comm_parameter.slave_id, mbus_map_element[ENUM_MBUS_REG_TAR_SPEED].addr, reg);
	if(ESP_OK != err)
	{
		ESP_LOGE(TAG, "Error in setting speed of motor");
		return err;
	}


	if((mbus_map_element[ENUM_MBUS_REG_START_STOP].enable == 1) &&
			(mbus_map_element[ENUM_MBUS_REG_START_STOP].parameter != 3))
	{
		switch((mbus_on_off_enum_type)mbus_map_element[ENUM_MBUS_REG_START_STOP].parameter)
		{
		case ENUM_MOTOR_ON_0_1:
		{
			valid = 1;
			if(value == 1){ step1 = 0; 	step2 = 1;}
			else { step1 = 1; 	step2 = 0;}
		}
		break;
		case ENUM_MOTOR_ON_1_0:
		{
			valid = 1;
			if(value == 1){ step1 = 1; 	step2 = 0;}
			else { step1 = 0; 	step2 = 1;}
		}
		break;

		default: valid = 0; break;
		}



	}

	if(valid)
	{
		reg.word = step1;
		ESP_LOGE("--->", "Slave id = %d  Addr = %d", mbus_comm_parameter.slave_id, mbus_map_element[ENUM_MBUS_REG_START_STOP].addr);
		err = modbus_write_holding_reg(mbus_comm_parameter.slave_id, mbus_map_element[ENUM_MBUS_REG_START_STOP].addr, reg);
		if(ESP_OK != err)
		{
			ESP_LOGI(TAG, "Error in starting motor");
			return err;
		}

		reg.word = step2;
		ESP_LOGE("--->", "Slave id = %d  Addr = %d", mbus_comm_parameter.slave_id, mbus_map_element[ENUM_MBUS_REG_START_STOP].addr);
		err = modbus_write_holding_reg(mbus_comm_parameter.slave_id, mbus_map_element[ENUM_MBUS_REG_START_STOP].addr, reg);
		if(ESP_OK != err)
		{
			ESP_LOGI(TAG, "Error in starting motor");
			return err;
		}
	}
	return ESP_OK;
}

esp_err_t modbus_get_actual_speed(uint16_t * speed)
{
	esp_err_t err;
	mbus_reg_type reg;


	ESP_LOGE("--->", "Slave id = %d  Addr = %d", mbus_comm_parameter.slave_id, mbus_map_element[ENUM_MBUS_REG_ACT_SPEED].addr);
	err = modbus_read_holding_reg(mbus_comm_parameter.slave_id, mbus_map_element[ENUM_MBUS_REG_ACT_SPEED].addr, 1, &reg);
	if(ESP_OK != err)
	{
		ESP_LOGE(TAG, "Error in getting motor speed");
		return err;
	}


	*speed = reg.word;
	ESP_LOGI("--->", "Mbus speed = %d %d", *speed, reg.word);
	return ESP_OK;
}

esp_err_t modbus_get_power_info(uint16_t * power_details)
{
	esp_err_t err;
	mbus_reg_type reg;

	err = modbus_read_holding_reg(mbus_comm_parameter.slave_id, mbus_map_element[ENUM_MBUS_REG_PWR].addr, 1, &reg);
	if(ESP_OK != err)
	{
		ESP_LOGE(TAG, "Error in getting motor speed");
		*power_details = 0;
		return err;
	}

	*power_details = reg.word;
	ESP_LOGI("--->", "Mbus pwr = %u", *power_details);
	return ESP_OK;
}
