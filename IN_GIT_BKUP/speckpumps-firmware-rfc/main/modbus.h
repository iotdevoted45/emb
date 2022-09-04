/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#ifndef MAIN_MODBUS_H_
#define MAIN_MODBUS_H_


#include "string.h"
#include "esp_log.h"
#include "mbcontroller.h"
#include "sdkconfig.h"

#include "mbcontroller.h"

#include "commondef.h"

#define MBUS_SL_ADDR			1
#define MBUS_ADDR_START			1
#define MBUS_ADDR_SPEED_TAR		2
#define MBUS_ADDR_SPEED_ACT		3
#define MBUS_SPEED_MAX			4500
#define MBUS_SPEED_MIN			300
#define MBUS_SPEED_DEF			3636


esp_err_t modbus_master_init();
esp_err_t modbus_write_holding_reg(uint8_t slave_addr, uint16_t addr, mbus_reg_type reg);
esp_err_t modbus_read_holding_reg(uint8_t slave_addr, uint16_t start_addr, uint8_t reg_count, mbus_reg_type *reg);

esp_err_t modbus_start_stop_motor(uint8_t value, uint16_t speed);
esp_err_t modbus_speed_change_motor(uint16_t new_speed);
esp_err_t modbus_get_actual_speed(uint16_t * speed);
esp_err_t modbus_get_power_info(uint16_t * power_details);


void task_modbus_scan(void *pV);


#endif /* MAIN_MODBUS_H_ */
