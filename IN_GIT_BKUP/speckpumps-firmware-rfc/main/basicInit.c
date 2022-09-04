/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#include "basicInit.h"

esp_err_t init_system()
{
	esp_err_t ret = ESP_FAIL;

	uint8_t mac[6];

	ret = esp_read_mac(mac, ESP_MAC_WIFI_STA);
	if( ret != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to read the Device's MAC Address");
		return ESP_FAIL;
	}

	sprintf(soft_ap_data.name, "%s%02X:%02X:%02X:%02X:%02X:%02X", AP_PREFIX, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	sprintf(soft_ap_data.name_short, "%s%02X%02X%02X", AP_PREFIX, mac[3], mac[4], mac[5]);
	sprintf(soft_ap_data.password, "%s", AP_PASS);

	ESP_LOGI(TAG, "Device name = %s", soft_ap_data.name);

	return ret;
}
