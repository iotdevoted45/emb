/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#include "fileOperations.h"

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"

const char file_scheduler[] = "/spiffs/scheduler";
const char file_operating_mode[] = "/spiffs/opmode";
const char file_device_type[] = "/spiffs/device_type";
const char file_modbus_map[] = "/spiffs/mbusmap";
const char file_modbus_param[] = "/spiffs/mbusparam";
const char file_prov_data[] = "/spiffs/prov";
const char file_timezone_data[] = "/spiffs/tz";
const char file_priming_data[] = "/spiffs/priming";
const char file_endpoint[] = "/spiffs/ep";
const char file_hb_data[] = "/spiffs/hbdata";

const char file_key[] = "/spiffs/key";
const char file_cert[] = "/spiffs/cert";

esp_err_t init_file_system()
{
	esp_err_t ret = ESP_FAIL;

	esp_vfs_spiffs_conf_t conf = {
			.base_path = "/spiffs",
			.partition_label = NULL,
			.max_files = 15,
			.format_if_mount_failed = false
	};

	ret = esp_vfs_spiffs_register(&conf);

	if(ret != ESP_OK)
	{
		if (ret == ESP_FAIL)
		{
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		}
		else if (ret == ESP_ERR_NOT_FOUND)
		{
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		}
		else
		{
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		}
	}
	return ret;
}

void read_provisioned_data()
{
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	FILE *fp;
	fp = fopen(file_prov_data, "rb");
	if(fp == NULL)
	{
		xSemaphoreGive(file_operation_mutex);
		return;
	}
	fread(&prov_data, sizeof(prov_data_def), 1, fp);
	fclose(fp);
	xSemaphoreGive(file_operation_mutex);
	printf("PROV: [%d]\n", prov_data.isProvisioned);
	printf("SSID: [%s]\n", prov_data.ssid);
	printf("PASS: [%s]\n", prov_data.password);
}

esp_err_t update_provisioned_data()
{
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	FILE *fp;

	fp = fopen(file_prov_data, "wb");
	if(fp == NULL)
	{
		xSemaphoreGive(file_operation_mutex);
		return ESP_FAIL;
	}
	fwrite(&prov_data, sizeof(prov_data_def), 1, fp);
	fclose(fp);
	xSemaphoreGive(file_operation_mutex);
	return ESP_OK;
}

void clear_provisioned_data()
{
	memset((void *)&prov_data, 0, sizeof(prov_data_def));
	update_provisioned_data();
}


void read_priming_data()
{
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	FILE *fp;
	fp = fopen(file_priming_data, "rb");
	if(fp == NULL)
	{
		xSemaphoreGive(file_operation_mutex);
		return;
	}
	fread(&priming_data, sizeof(priming_data), 1, fp);
	fclose(fp);
	xSemaphoreGive(file_operation_mutex);
	ESP_LOGI(TAG, "Priming Enable: [%d]\n", priming_data.is_enabled);
	ESP_LOGI(TAG, "Priming Speed: [%d]\n", priming_data.priming_speed);
	ESP_LOGI(TAG, "Priming Time: [%ld]\n", priming_data.priming_time);
}

esp_err_t update_priming_data()
{
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	FILE *fp;

	fp = fopen(file_priming_data, "wb");
	if(fp == NULL)
	{
		xSemaphoreGive(file_operation_mutex);
		return ESP_FAIL;
	}
	fwrite(&priming_data, sizeof(priming_data_def), 1, fp);
	fclose(fp);
	xSemaphoreGive(file_operation_mutex);
	return ESP_OK;
}

void clear_priming_data()
{
	memset((void *)&priming_data, 0, sizeof(priming_data_def));
	update_priming_data();
}


void read_time_zone()
{
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	FILE *fp;
	fp = fopen(file_timezone_data, "rb");
	if(fp == NULL)
	{
		xSemaphoreGive(file_operation_mutex);
		return;
	}
	fread(&device_time_zone, sizeof(time_zone_type), 1, fp);
	fclose(fp);
	xSemaphoreGive(file_operation_mutex);
	ESP_LOGI(TAG, "Device time zone = [%s]", device_time_zone.time_zone);
}

esp_err_t update_time_zone()
{
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	FILE *fp;
	fp = fopen(file_timezone_data, "wb");
	if(fp == NULL)
	{
		xSemaphoreGive(file_operation_mutex);
		return ESP_FAIL;
	}
	fwrite(&device_time_zone, sizeof(time_zone_type), 1, fp);
	fclose(fp);
	setenv("TZ", device_time_zone.time_zone, 1);
	tzset();
	xSemaphoreGive(file_operation_mutex);
	return ESP_OK;
}

void init_scheduler()
{
	long int filesize;
	sched_def sched;
	size_t bytes;
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	FILE *fp;
	fp = fopen(file_scheduler, "a");
	if(fp == NULL)
	{
		xSemaphoreGive(file_operation_mutex);
		return;
	}
	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	ESP_LOGE(TAG, "File size = %ld %d %d %d", filesize, MAX_SCHED, sizeof(sched_def), MAX_SCHED*sizeof(sched_def));
	if( filesize != (MAX_SCHED*sizeof(sched_def)))
	{
		fclose(fp);
		fp = fopen(file_scheduler, "w");
		if(fp == NULL)
		{
			xSemaphoreGive(file_operation_mutex);
			return;
		}
		ESP_LOGE(TAG, "Scheduler file needs to create");
		fseek(fp, 0, SEEK_SET);
		memset(&sched, 0, sizeof(sched_def));
		for(int i = 0 ; i < MAX_SCHED; i++)
			bytes = fwrite(&sched, 1, sizeof(sched_def), fp);
		ESP_LOGE(TAG,"Bytes written = %d\n", bytes);
	}
	else
	{
		ESP_LOGE(TAG,"Scheduler file exists");
	}
	fclose(fp);
	xSemaphoreGive(file_operation_mutex);
}

void scheduler_read(uint8_t number, sched_def *sched)
{
	if((number < 1) || (number > MAX_SCHED))
	{
		ESP_LOGE(TAG, "Error in scheduler argument");
		memset((void *)sched, 0, sizeof(sched_def));
		return;
	}
	number--;
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	FILE *fp;
	fp = fopen(file_scheduler, "rb");
	if(fp == NULL)
	{
		ESP_LOGE(TAG, "Failed to open scheduler file");
		memset((void *)sched, 0, sizeof(sched_def));
		xSemaphoreGive(file_operation_mutex);
		return;
	}
	fseek(fp, number*sizeof(sched_def), SEEK_SET);
	fread((void *)sched, 1, sizeof(sched_def), fp);
	fclose(fp);
	xSemaphoreGive(file_operation_mutex);
}

void scheduler_update(uint8_t number, sched_def sched)
{
	size_t bytes;
	if((number < 1) || (number > MAX_SCHED))
	{
		ESP_LOGE(TAG, "Error in scheduler argument");
		return;
	}
	number--;
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	FILE *fp;
	fp = fopen(file_scheduler, "rb+");
	if(fp == NULL)
	{
		ESP_LOGE(TAG, "Failed to open scheduler file");
		xSemaphoreGive(file_operation_mutex);
		return;
	}

	fseek(fp, number*sizeof(sched_def), SEEK_SET);
	ESP_LOGI(TAG, "Position = %ld", ftell(fp));
	bytes = fwrite((void *)&sched, 1, sizeof(sched_def), fp);
	ESP_LOGE(TAG, "Bytes written = %d", bytes);
	fclose(fp);

	xSemaphoreGive(file_operation_mutex);
}



void _write_operating_mode(const char *filename, uint8_t mode)
{
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);

	FILE *fp;
	fp = fopen(filename, "wb");
	if(fp == NULL)
	{
		ESP_LOGE(TAG, "Failed to open operating mode file");
		xSemaphoreGive(file_operation_mutex);

		return;
	}

	size_t bytes = fwrite((void *)&mode, 1, 1, fp);
	ESP_LOGE("-->", "Bytes written in mode file = %d", bytes);
	fclose(fp);
	xSemaphoreGive(file_operation_mutex);
}

void read_operating_mode()
{
	uint8_t temp;
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	FILE *fp;
	fp = fopen(file_operating_mode, "rb");
	if(fp == NULL)
	{
		if(device_operation_mode == ENUM_OP_MODE_CLEAN)
		{
			device_operation_mode = ENUM_OP_MODE_CLEAN;
		}
		else if(device_operation_mode == ENUM_OP_MODE_AUTO)
		{
			device_operation_mode = ENUM_OP_MODE_AUTO;
		}
		else
		{
			device_operation_mode = ENUM_OP_MODE_MANUAL;
		}
		xSemaphoreGive(file_operation_mutex);

		_write_operating_mode(file_operating_mode, device_operation_mode);
		return;
	}
	fseek(fp, 0, SEEK_END);
	int long lastPost = ftell(fp);
	ESP_LOGI(TAG, "Position = %ld", lastPost);
	if(lastPost > 0)
	{
		lastPost--;
		fseek(fp, lastPost, SEEK_SET);
	}
	size_t bytes = fread((void *)&temp, 1, 1, fp);
	ESP_LOGE(TAG, "Bytes read in operating file = %d", bytes);
	device_operation_mode = temp;
	xSemaphoreGive(file_operation_mutex);
}

void update_operating_mode()
{
	uint8_t temp;
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);

	FILE *fp;
	fp = fopen(file_operating_mode, "rb+");
	if(fp == NULL)
	{
		if(device_operation_mode == ENUM_OP_MODE_AUTO)
		{
			device_operation_mode = ENUM_OP_MODE_AUTO;
		}
		else if(device_operation_mode == ENUM_OP_MODE_MANUAL)
		{
			device_operation_mode = ENUM_OP_MODE_MANUAL;
		}
		else if(device_operation_mode == ENUM_OP_MODE_CLEAN)
		{
			device_operation_mode = ENUM_OP_MODE_CLEAN;
		}
		else
			device_operation_mode = ENUM_OP_MODE_MANUAL;

		xSemaphoreGive(file_operation_mutex);
		_write_operating_mode(file_operating_mode, device_operation_mode);
		return;
	}
	fseek(fp, 0, SEEK_END);
	long int filesize;
	filesize = ftell(fp);
	ESP_LOGI("Update", "File size = %ld", filesize);
	temp = device_operation_mode;
	if(filesize < 500)
	{
		size_t bytes = fwrite((void *)&temp, 1, 1, fp);
		ESP_LOGE(TAG, "Bytes write in operating file = %d operating mode = %d", bytes, device_operation_mode);
		fclose(fp);
		xSemaphoreGive(file_operation_mutex);
	}
	else
	{
		fclose(fp);
		xSemaphoreGive(file_operation_mutex);
		_write_operating_mode(file_operating_mode, temp);
	}
}

//-----------------------------------------

void read_device_type()
{
	uint8_t temp;
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	FILE *fp;
	fp = fopen(file_device_type, "rb");
	if(fp == NULL)
	{
		if(device_type == ENUM_DEVICE_TYPE_PFC)
		{
			device_type = ENUM_DEVICE_TYPE_PFC;
		}
		else
		{
			device_type = ENUM_DEVICE_TYPE_RS485;
		}
		xSemaphoreGive(file_operation_mutex);

		_write_operating_mode(file_device_type, device_type);
		return;
	}
	fseek(fp, 0, SEEK_END);
	int long lastPost = ftell(fp);
	ESP_LOGI(TAG, "Position = %ld", lastPost);
	if(lastPost > 0)
	{
		lastPost--;
		fseek(fp, lastPost, SEEK_SET);
	}
	size_t bytes = fread((void *)&temp, 1, 1, fp);
	ESP_LOGE(TAG, "Bytes read in device type file = %d", bytes);
	device_type = temp;
	xSemaphoreGive(file_operation_mutex);
}

void update_device_type()
{
	uint8_t temp;
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);

	FILE *fp;
	fp = fopen(file_device_type, "rb+");
	if(fp == NULL)
	{
		if(device_type == ENUM_DEVICE_TYPE_PFC)
		{
			device_type = ENUM_DEVICE_TYPE_PFC;
		}
		else
			device_type = ENUM_DEVICE_TYPE_RS485;

		xSemaphoreGive(file_operation_mutex);
		_write_operating_mode(file_device_type, device_type);
		return;
	}
	fseek(fp, 0, SEEK_END);
	long int filesize;
	filesize = ftell(fp);
	ESP_LOGI("Update", "File size = %ld", filesize);
	temp = device_type;
	if(filesize < 500)
	{
		size_t bytes = fwrite((void *)&temp, 1, 1, fp);
		ESP_LOGE(TAG, "Bytes write in operating file = %d operating mode = %d", bytes, device_type);
		fclose(fp);
		xSemaphoreGive(file_operation_mutex);
	}
	else
	{
		fclose(fp);
		xSemaphoreGive(file_operation_mutex);
		_write_operating_mode(file_device_type, temp);
	}
}

//------------------------------------------------------------------
void read_mbus_map()
{
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	FILE *fp;
	fp = fopen(file_modbus_map, "rb");
	if(fp == NULL)
	{
		xSemaphoreGive(file_operation_mutex);
		return;
	}
	fread(mbus_map_element, sizeof(mbus_map_element_type)*MAX_MBUS_REG, 1, fp);
	fclose(fp);
	xSemaphoreGive(file_operation_mutex);
}

esp_err_t save_mbus_map()
{
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	FILE *fp;
	fp = fopen(file_modbus_map, "wb");
	if(fp == NULL)
	{
		xSemaphoreGive(file_operation_mutex);
		return ESP_FAIL;
	}
	fwrite(mbus_map_element, sizeof(mbus_map_element_type)*MAX_MBUS_REG, 1, fp);
	fclose(fp);
	xSemaphoreGive(file_operation_mutex);
	return ESP_OK;
}

void read_mbus_comm_param()
{
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	FILE *fp;
	fp = fopen(file_modbus_param, "rb");
	if(fp == NULL)
	{
		xSemaphoreGive(file_operation_mutex);
		mbus_comm_parameter.baud_rate = 9600;
		mbus_comm_parameter.slave_id = 1;
		return;
	}
	fread(&mbus_comm_parameter, sizeof(mbus_comm_parameter_type), 1, fp);
	fclose(fp);
	xSemaphoreGive(file_operation_mutex);
}

esp_err_t save_mbus_comm_param()
{
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	FILE *fp;
	fp = fopen(file_modbus_param, "wb");
	if(fp == NULL)
	{
		xSemaphoreGive(file_operation_mutex);
		return ESP_FAIL;
	}
	fwrite(&mbus_comm_parameter, sizeof(mbus_comm_parameter_type), 1, fp);
	fclose(fp);
	xSemaphoreGive(file_operation_mutex);
	return ESP_OK;
}


void get_key(char *data, ssize_t len, ssize_t *p_cert_len)
{
	FILE *fp;
	ssize_t filesize;
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	struct stat st;
	if(stat(file_key, &st) == 0)
	{
		filesize = st.st_size;
	}
	else
	{
		ESP_LOGE(TAG, "Failed to get info for key file");
		xSemaphoreGive(file_operation_mutex);
		return;
	}

	fp = fopen(file_key, "r");
	if(fp == NULL)
	{
		ESP_LOGE(TAG, "Failed to open key file");
		xSemaphoreGive(file_operation_mutex);
		return;
	}


	if(filesize < len)
	{
		fseek(fp, 0, SEEK_SET);
		fread(data, filesize, 1, fp);
	}
	else
	{
		ESP_LOGE(TAG, "key file read: Buffer size issue");
	}

	fclose(fp);
	xSemaphoreGive(file_operation_mutex);
	*p_cert_len = filesize;
}

void get_cert(char *data, ssize_t len, ssize_t *p_cert_len)
{
	FILE *fp;
	ssize_t filesize;
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	struct stat st;
	if(stat(file_cert, &st) == 0)
	{
		filesize = st.st_size;
	}
	else
	{
		ESP_LOGE(TAG, "Failed to get info for cert file");
		xSemaphoreGive(file_operation_mutex);
		return;
	}

	fp = fopen(file_cert, "r");
	if(fp == NULL)
	{
		ESP_LOGE(TAG, "Failed to open cert file");
		xSemaphoreGive(file_operation_mutex);
		return;
	}



	if(filesize < len)
	{
		fseek(fp, 0, SEEK_SET);
		fread(data, 1, filesize, fp);
	}
	else
	{
		ESP_LOGE(TAG, "Cert file read: Buffer size issue");
	}

	fclose(fp);
	xSemaphoreGive(file_operation_mutex);
	*p_cert_len = filesize;
}


void get_endpoint(char *data, ssize_t len, ssize_t *p_endpoint_lenght)
{
	FILE *fp;
	ssize_t filesize;
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	struct stat st;
	if (stat(file_endpoint, &st) == 0)
	{
		filesize = st.st_size;
	}
	else
	{
		ESP_LOGE(TAG, "Failed to get info for endpoint");
		xSemaphoreGive(file_operation_mutex);
		return;
	}

	fp = fopen(file_endpoint, "r");
	if (fp == NULL)
	{
		ESP_LOGE(TAG, "Failed to open endpoint file");
		xSemaphoreGive(file_operation_mutex);
		return;
	}

	if (filesize < len)
	{
		fseek(fp, 0, SEEK_SET);
		fread(data, 1, filesize, fp);
	}
	else
	{
		ESP_LOGE(TAG, "Cert file read: Buffer size issue");
	}

	fclose(fp);
	xSemaphoreGive(file_operation_mutex);
	*p_endpoint_lenght = filesize;
}

void save_endpoint(char *data, ssize_t len)
{
	FILE *fp;
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	fp = fopen(file_endpoint, "w");
	if (fp == NULL)
	{
		ESP_LOGE(TAG, "Failed to write cert file");
		xSemaphoreGive(file_operation_mutex);
		return;
	}
	fwrite(data, len, 1, fp);
	fclose(fp);
	xSemaphoreGive(file_operation_mutex);
}

void read_hb_data()
{
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	FILE *fp;
	fp = fopen(file_hb_data, "rb");
	if(fp == NULL)
	{
		xSemaphoreGive(file_operation_mutex);
		return;
	}
	fread(&hb_data, sizeof(hb_data_type), 1, fp);
	fclose(fp);
	xSemaphoreGive(file_operation_mutex);
	ESP_LOGI(TAG, "HB Data  Enable = %d Interval = %d", hb_data.enable, hb_data.interval);
}

esp_err_t update_hb_data()
{
	xSemaphoreTake(file_operation_mutex, portMAX_DELAY);
	FILE *fp;
	fp = fopen(file_hb_data, "wb");
	if(fp == NULL)
	{
		xSemaphoreGive(file_operation_mutex);
		return ESP_FAIL;
	}
	fwrite(&hb_data, sizeof(hb_data_type), 1, fp);
	fclose(fp);
	ESP_LOGI(TAG, "HB Data Update Enable = %d Interval = %d", hb_data.enable, hb_data.interval);

	xSemaphoreGive(file_operation_mutex);
	return ESP_OK;
}

