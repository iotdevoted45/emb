/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#include "i2c_peri.h"

#include <string.h>

#define I2C_WRITE_BIT		0x00    //I2C master write
#define I2C_READ_BIT		0x01    // I2C master read
#define I2C_ACK_CHECK_EN 	0x01    // I2C master will check ack from slave
#define I2C_ACK_CHECK_DIS 	0x00    // I2C master will not check ack from slave
#define I2C_ACK_VAL 		0x00    // I2C ack value
#define I2C_NACK_VAL 		0x01    // I2C nack value

//------------- MCP79410 ----------------------------------//

#define	MCP79410_ADDR				(0xDE)

#define MCP79410_REG_Sec			(0x00)
#define MCP79410_REG_Min			(0x01)
#define MCP79410_REG_Hour			(0x02)
#define MCP79410_REG_WDay			(0x03)
#define MCP79410_REG_Date			(0x04)
#define MCP79410_REG_Month			(0x05)
#define MCP79410_REG_Year			(0x06)

#define MCP79410_REG_Control		(0x07)
#define MCP79410_REG_OscTrim		(0x08)
#define MCP79410_REG_EEUnlock		(0x09)

#define MCP79410_REG_Alm0_Sec		(0x0A)
#define MCP79410_REG_Alm0_Min		(0x0B)
#define MCP79410_REG_Alm0_Hour		(0x0C)
#define MCP79410_REG_Alm0_WDay		(0x0D)
#define MCP79410_REG_Alm0_Date		(0x0E)
#define MCP79410_REG_Alm0_Month		(0x0F)

#define MCP79410_REG_Alm1_Sec		(0x11)
#define MCP79410_REG_Alm1_Min		(0x12)
#define MCP79410_REG_Alm1_Hour		(0x13)
#define MCP79410_REG_Alm1_WDay		(0x14)
#define MCP79410_REG_Alm1_Date		(0x15)
#define MCP79410_REG_Alm1_Month		(0x16)

#define MCP79410_REG_PWRDN_Min		(0x18)
#define MCP79410_REG_PWRDN_Hour		(0x19)
#define MCP79410_REG_PWRDN_Date		(0x1A)
#define MCP79410_REG_PWRDN_Month 	(0x1B)

#define MCP79410_REG_PWRUP_Min		(0x1C)
#define MCP79410_REG_PWRUP_Hour		(0x1D)
#define MCP79410_REG_PWRUP_Date		(0x1E)
#define MCP79410_REG_PWRUP_Month 	(0x1F)



uint8_t decToBCD(uint8_t x);
uint8_t bcdToDEC(uint8_t x);

esp_err_t i2c_master_init(void)
{
	int i2c_master_port = I2C_NUM_0;
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = I2C_SDA_IO;
	conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
	conf.scl_io_num = I2C_SCL_IO;
	conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
	conf.master.clk_speed = I2C_FREQ;
	i2c_param_config(i2c_master_port, &conf);
	return i2c_driver_install(i2c_master_port, conf.mode,0, 0, 0);
}

esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t slave_addr, uint8_t *data_rd, size_t size)
{
#if I2C_DUMMY
	return ESP_OK;
#endif

	if (size == 0)
	{
		return ESP_OK;
	}

	xSemaphoreTake(mutex_i2c, portMAX_DELAY);
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (slave_addr ) | I2C_READ_BIT, I2C_ACK_CHECK_EN);
	if (size > 1)
	{
		i2c_master_read(cmd, data_rd, size - 1, I2C_ACK_VAL);
	}
	i2c_master_read_byte(cmd, data_rd + size - 1, I2C_NACK_VAL);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	xSemaphoreGive(mutex_i2c);
	return ret;
}

esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t slave_addr, uint8_t *data_wr, size_t size)
{
#if I2C_DUMMY
	return ESP_OK;
#endif
	xSemaphoreTake(mutex_i2c, portMAX_DELAY);
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (slave_addr ) | I2C_WRITE_BIT, I2C_ACK_CHECK_EN);
	i2c_master_write(cmd, data_wr, size, I2C_ACK_CHECK_EN);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	xSemaphoreGive(mutex_i2c);
	return ret;
}

esp_err_t i2c_master_read_reg(i2c_port_t i2c_num, uint8_t slave_addr, uint8_t reg_addr, uint8_t *data_rd, size_t size)
{
#if I2C_DUMMY
	return ESP_OK;
#endif
	if (size == 0) {
		return ESP_OK;
	}

	esp_err_t ret;

	xSemaphoreTake(mutex_i2c, portMAX_DELAY);
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (slave_addr ) | I2C_WRITE_BIT, I2C_ACK_CHECK_EN);
	i2c_master_write_byte(cmd, reg_addr, I2C_ACK_CHECK_EN);
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	if(ret != ESP_OK)
	{
		xSemaphoreGive(mutex_i2c);
		return ret;
	}
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (slave_addr ) | I2C_READ_BIT, I2C_ACK_CHECK_EN);
	if (size > 1) {
		i2c_master_read(cmd, data_rd, size - 1, I2C_ACK_VAL);
	}
	i2c_master_read_byte(cmd, data_rd + size - 1, I2C_NACK_VAL);
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);


	xSemaphoreGive(mutex_i2c);
	return ret;
}

esp_err_t i2c_master_write_reg(i2c_port_t i2c_num, uint8_t slave_addr, uint8_t reg_addr, uint8_t *data_wr, size_t size)
{
#if I2C_DUMMY
	return ESP_OK;
#endif
	xSemaphoreTake(mutex_i2c, portMAX_DELAY);
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (slave_addr ) | I2C_WRITE_BIT, I2C_ACK_CHECK_EN);
	i2c_master_write_byte(cmd, reg_addr, I2C_ACK_CHECK_EN);
	i2c_master_write(cmd, data_wr, size, I2C_ACK_CHECK_EN);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	xSemaphoreGive(mutex_i2c);
	return ret;
}

esp_err_t init_MCP79410()
{

	uint8_t u8Temp;

	//Set the control register
	u8Temp = 0xC3;
	if(ESP_OK != i2c_master_write_reg(I2C_NUM_0, MCP79410_ADDR, MCP79410_REG_Control, &u8Temp, 1))
	{
		ESP_LOGE(TAG, "Error in initialization of MCP79410 : 1");
		return ESP_FAIL;
	}

	//Battery enable
	u8Temp = 0x00;
	if(ESP_OK != i2c_master_read_reg(I2C_NUM_0, MCP79410_ADDR, MCP79410_REG_WDay, &u8Temp, 1))
	{
		ESP_LOGE(TAG, "Error in initialization of MCP79410 : 2");
		return ESP_FAIL;
	}
	u8Temp |= (1 << 3);
	if(ESP_OK != i2c_master_write_reg(I2C_NUM_0, MCP79410_ADDR, MCP79410_REG_WDay, &u8Temp, 1))
	{
		ESP_LOGE(TAG, "Error in initialization of MCP79410 : 3");
		return ESP_FAIL;
	}

	//Oscillator enable
	u8Temp = 0x00;
	if(ESP_OK != i2c_master_read_reg(I2C_NUM_0, MCP79410_ADDR, MCP79410_REG_Sec, &u8Temp, 1))
	{
		ESP_LOGE(TAG, "Error in initialization of MCP79410 : 4");
		return ESP_FAIL;
	}

	ESP_LOGE(TAG, "MCP79410  Read %x", u8Temp);
	if(u8Temp & 0x80)
	{
		return ESP_OK;
	}
	else
	{
		u8Temp |= 0x80;
		ESP_LOGE(TAG, "MCP79410  Read %x", u8Temp);
		if(ESP_OK != i2c_master_write_reg(I2C_NUM_0, MCP79410_ADDR, MCP79410_REG_Sec, &u8Temp, 1))
		{
			ESP_LOGE(TAG, "Error in initialization of MCP79410 : 5");
			return ESP_FAIL;
		}
	}
	return ESP_OK;
}

uint8_t dec2bcd(uint8_t x)
{
	return (((x/10) << 4) | ((x%10) & 0x0F));
}

uint8_t bcd2dec(uint8_t x)
{
	return ((x >> 4)*10 + (x & 0x0F));
}

esp_err_t MCP79410_RTC_getTime(struct tm *time)
{
	ESP_LOGI("-->", "Reading MCP79410");

	uint8_t data[7];
	if(ESP_OK != i2c_master_read_reg(I2C_NUM_0, MCP79410_ADDR, MCP79410_REG_Sec, data, 7))
	{
		ESP_LOGE(TAG, "Error in reading MCP79410");
		return ESP_FAIL;
	}

	/*convert MCP79410 data into time structure*/
	ESP_LOGI("-->", "Reading MCP79410");
	time->tm_sec = bcd2dec(data[0]);
	time->tm_min = bcd2dec(data[1]);
	time->tm_hour = bcd2dec(data[2]);
	time->tm_wday = bcd2dec(data[3])-1;
	time->tm_mday = bcd2dec(data[4]);
	time->tm_mon = bcd2dec(data[5])-1;
	time->tm_year = bcd2dec(data[6])+100;
	time->tm_isdst = 0;

	ESP_LOGI("-->", "%d", time->tm_year);

	return ESP_OK;
}

esp_err_t MCP79410_RTC_setTime(struct tm *time)
{
	ESP_LOGI(TAG, "Sec = %d", time->tm_sec);
	ESP_LOGI(TAG, "Min = %d", time->tm_min);
	ESP_LOGI(TAG, "Hrs = %d", time->tm_hour);
	ESP_LOGI(TAG, "day = %d", time->tm_wday);
	ESP_LOGI(TAG, "Date = %d", time->tm_mday);
	ESP_LOGI(TAG, "Month = %d", time->tm_mon);
	ESP_LOGI(TAG, "year = %d", time->tm_year);

	uint8_t data[7];

	data[0] = dec2bcd(time->tm_sec);
	data[1] = dec2bcd(time->tm_min);

	data[2] = dec2bcd(time->tm_hour);

	data[3] = dec2bcd(time->tm_wday + 1);
	data[4] = dec2bcd(time->tm_mday);
	data[5] = dec2bcd(time->tm_mon+1);
	data[6] = dec2bcd(time->tm_year%100);

	if(ESP_OK != i2c_master_write_reg(I2C_NUM_0, MCP79410_ADDR, MCP79410_REG_Sec, data, 7))
	{
		ESP_LOGE(TAG, "Error in writing data in MCP79410");
		return ESP_FAIL;
	}

	return ESP_OK;
}
