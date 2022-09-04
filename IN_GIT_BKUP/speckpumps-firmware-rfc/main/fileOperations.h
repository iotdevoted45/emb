/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#ifndef MAIN_FILEOPERATIONS_H_
#define MAIN_FILEOPERATIONS_H_

#include "esp_err.h"
#include "esp_system.h"
#include "esp_spiffs.h"
#include "esp_log.h"
#include "wifiCustomStruct.h"
#include "commondef.h"

esp_err_t init_file_system();

void read_provisioned_data();
esp_err_t update_provisioned_data();
void clear_provisioned_data();

void read_time_zone();
esp_err_t update_time_zone();

void init_scheduler();
void scheduler_read(uint8_t number, sched_def *sched);
void scheduler_update(uint8_t number, sched_def sched);

void read_operating_mode();
void update_operating_mode();

void read_device_type();
void update_device_type();

void read_mbus_map();
esp_err_t save_mbus_map();
void read_mbus_comm_param();
esp_err_t save_mbus_comm_param();

void read_priming_data();
esp_err_t update_priming_data();
void clear_priming_data();

void get_key(char *data, ssize_t len, ssize_t *p_cert_len);
void get_cert(char *data, ssize_t len, ssize_t *p_cert_len);

void get_endpoint(char *data, ssize_t len, ssize_t *p_endpoint_lenght);
void save_endpoint(char *data, ssize_t len);

void read_hb_data();
esp_err_t update_hb_data();

#endif /* MAIN_FILEOPERATIONS_H_ */
