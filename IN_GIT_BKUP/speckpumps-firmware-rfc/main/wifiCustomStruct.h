/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#ifndef MAIN_WIFICUSTOMSTRUCT_H_
#define MAIN_WIFICUSTOMSTRUCT_H_

#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define AP_PREFIX	"SPECK_"
#define AP_PASS		"password"

typedef struct __xProvData__
{
	char isProvisioned;
	char ssid[33];
	char password[65];
}prov_data_def;
prov_data_def prov_data;

typedef struct __xSoftAPDataType__
{
	char name[33];
	char password[65];
	char name_short[33];
}soft_ap_data_def;
soft_ap_data_def soft_ap_data;


#endif /* MAIN_WIFICUSTOMSTRUCT_H_ */
