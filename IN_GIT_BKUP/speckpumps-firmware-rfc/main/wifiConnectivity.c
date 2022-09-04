/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#include <string.h>
#include "commondef.h"
#include "rtc_time.h"
#include "commondef.h"


#include "wifiConnectivity.h"
#define CPOSTBUFSIZE	512

const char key_w_device_type[] = "device_type";
const char key_w_slave_id[] = "slave_id";
const char key_w_baud_rate[] = "baud_rate";
const char key_w_id[] = "id";
const char key_w_address[] = "address";
const char key_w_enable[] = "enable";
const char key_w_param[] = "param";

uint8_t disconnect_count = 0;

time_t current_unix_time;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data);
static void ip_event_handler(void * arg, esp_event_base_t event_base,int32_t event_id, void* event_data);
void start_provisioning_server();

typedef enum __state__
{
	prov_start,
	prov_start_ap,
	prov_start_prov_server,
	prov_wait_to_start_scan,
	prov_start_scan,
	prov_scanning,
	prov_done_scanning,
	prov_wait_for_cred,
	prov_conecting_network,
	prov_connected_to_router,
	prov_wrong_cred,
	prov_done

}xState;

xState xStateCurrent, xStateNext, xStateCMD;

static httpd_handle_t xServer = NULL;

uint8_t u8ApListEntry = 0;
wifi_ap_record_t *xApList;
char cPostReqBuf[CPOSTBUFSIZE];


esp_err_t init_wifi()
{
	esp_err_t ret = nvs_flash_init();

	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ret = nvs_flash_erase();
		if(ret != ESP_OK)
		{
			ESP_LOGE(TAG, "Failed to erase NVS Flash");
			return ret;
		}
		ret = nvs_flash_init();
	}


	return ret;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
	switch(event_id)
	{
	case WIFI_EVENT_STA_START:
		ESP_LOGI(TAG, "STA mode started");

		xEventGroupClearBits(common_events_group, 1 << enum_connected_to_internet);

		xEventGroupClearBits(common_events_group, 1 << enum_wifi_connected);
		xEventGroupClearBits(common_events_group, 1 << enum_wifi_disconnected);
		if(xEventGroupGetBits(common_events_group) & (1 << enum_ap_on))
		{

		}
		else
		{
			ESP_ERROR_CHECK(esp_wifi_connect());
		}
		break;

	case WIFI_EVENT_STA_DISCONNECTED:
		ESP_LOGI(TAG, "STA mode disconnected");
		time(&current_unix_time);
		if( (motor_port.stop_time == 0) || (motor_port.stop_time < current_unix_time) || (motor_port.on_target == 0))
		{
			disconnect_count++;
			ESP_LOGI(TAG, "Disconnection count = %d", disconnect_count);
			if(disconnect_count > 200)
			{

				esp_wifi_stop();
				esp_wifi_deinit();
				esp_restart();
			}
		}

		xEventGroupClearBits(common_events_group, 1 << enum_connected_to_internet);

		xEventGroupClearBits(common_events_group, 1 << enum_wifi_connected);
		xEventGroupClearBits(common_events_group, 1 << enum_wifi_connecting);
		xEventGroupSetBits(common_events_group, 1 << enum_wifi_disconnected);
		esp_wifi_connect();
		break;

	case WIFI_EVENT_AP_START:
		ESP_LOGI(TAG, "AP mode started, Device is in AP mode");
		xEventGroupSetBits(common_events_group, 1 << enum_ap_on);
		start_provisioning_server();
		break;

	case WIFI_EVENT_AP_STOP:
		ESP_LOGI(TAG, "AP mode is stopped");
		break;

	case WIFI_EVENT_AP_STACONNECTED:
		ESP_LOGI(TAG, "AP mode is connected");
		break;

	case WIFI_EVENT_AP_STADISCONNECTED:
		ESP_LOGI(TAG, "AP mode is disconnected");
		break;

	default:
		break;
	}
}
static void ip_event_handler(void * arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
	uint8_t rssi;
	ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
	memset(ip_address_str, 0, sizeof(ip_address_str));
	sprintf(ip_address_str, "%s", ip4addr_ntoa((const ip4_addr_t *)&(event->ip_info.ip)));
	ESP_LOGI(TAG, "Device IP-Addr = [ %s ]", ip_address_str);
	xEventGroupClearBits(common_events_group, 1 << enum_wifi_disconnected);
	xEventGroupClearBits(common_events_group, 1 << enum_wifi_connecting);
	xEventGroupSetBits(common_events_group, 1 << enum_wifi_connected);

	xEventGroupSetBits(common_events_group, 1 << enum_connected_to_internet);
	get_rssi_info(&rssi);
	disconnect_count = 0;
}

wifi_config_t wifi_config;
wifi_config_t wifi_configSTA;
wifi_scan_config_t scan_config;

esp_err_t start_station()
{
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
			ESP_EVENT_ANY_ID,
			&wifi_event_handler,
			NULL,
			NULL));

	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
			IP_EVENT_STA_GOT_IP,
			&ip_event_handler,
			NULL,
			NULL));

	strcpy((char *)wifi_configSTA.sta.ssid, prov_data.ssid);
	strcpy((char *)wifi_configSTA.sta.password, prov_data.password);

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configSTA) );
	ESP_ERROR_CHECK(esp_wifi_start());


	return ESP_OK;
}

esp_err_t start_provisioning()
{
	esp_err_t ret = ESP_FAIL;

	xEventGroupSetBits(common_events_group, 1 << enum_ap_on);

	// configure and run the scan process in blocking mode
	scan_config.ssid = 0;
	scan_config.bssid = 0;
	scan_config.channel = 0;
	scan_config.show_hidden = true;

	strcpy((char *)wifi_config.ap.ssid, soft_ap_data.name);
	strcpy((char *)wifi_config.ap.password, soft_ap_data.password);
	wifi_config.ap.ssid_len = strlen(soft_ap_data.name);
	wifi_config.ap.max_connection = 4;
	wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;


	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_ap();
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
			ESP_EVENT_ANY_ID,
			&wifi_event_handler,
			NULL,
			NULL));

	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
			IP_EVENT_STA_GOT_IP,
			&ip_event_handler,
			NULL,
			NULL));

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));

	xStateCurrent = prov_start;
	xStateNext = prov_start;

	while(1)
	{
		switch(xStateCurrent)
		{
		case prov_start:
			ESP_ERROR_CHECK(esp_wifi_start());
			xStateNext = prov_wait_to_start_scan;
			ESP_ERROR_CHECK(esp_wifi_disconnect());
			break;

		case prov_wait_to_start_scan:
			if(xStateCMD == prov_start_scan)
				xStateNext = prov_start_scan;
			else if(xEventGroupGetBits(common_events_group) & (1 << enum_wifi_connecting))
			{
				xStateNext = prov_conecting_network;
			}
			else if(xEventGroupGetBits(common_events_group) & (1 << enum_wifi_disconnected))
			{
				xStateNext = prov_conecting_network;
			}
			else if(xEventGroupGetBits(common_events_group) & (1 << enum_wifi_connected))
			{
				xStateNext = prov_conecting_network;
			}
			else
				xStateNext = prov_wait_to_start_scan;
			break;

		case prov_start_scan:
			xStateCMD = 0;
			ESP_LOGI(TAG,"Scanning the available WIFI networks ...");
			ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
			ESP_LOGI(TAG,"Scanning is completed!");
			xStateNext = prov_done_scanning;
			break;

		case prov_done_scanning:
			if(u8ApListEntry != 0)
			{
				u8ApListEntry = 0;
				free(xApList);
			}
			esp_wifi_scan_get_ap_num((uint16_t *)&u8ApListEntry);
			xApList = malloc(u8ApListEntry * sizeof(wifi_ap_record_t));
			ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records((uint16_t *)&u8ApListEntry, xApList));
			xStateNext = prov_wait_for_cred;
			break;

		case prov_wait_for_cred:
			if(xStateCMD == prov_start_scan)
			{
				xStateCMD = 0;
				xStateNext = prov_start_scan;
			}
			else if(xEventGroupGetBits(common_events_group) & (1 << enum_wifi_connecting))
			{
				xStateNext = prov_conecting_network;
			}
			else if(xEventGroupGetBits(common_events_group) & (1 << enum_wifi_connected))
			{
				xStateNext = prov_conecting_network;
			}
			else if(xEventGroupGetBits(common_events_group) & (1 << enum_wifi_disconnected))
			{
				xStateNext = prov_conecting_network;
			}
			else
				xStateNext = prov_wait_for_cred;
			break;

		case prov_conecting_network:
			xStateNext = prov_conecting_network;
			if(xEventGroupGetBits(common_events_group) & (1 << enum_wifi_connected))
			{
				xStateNext = prov_connected_to_router;
			}
			if(xEventGroupGetBits(common_events_group) & (1 << enum_wifi_disconnected))
			{
				xStateNext = prov_wrong_cred;
			}
			break;

		case prov_connected_to_router:
			xStateNext = prov_connected_to_router;
			if(prov_data.isProvisioned == 0)
			{
				ESP_LOGI(TAG, "Saving Wifi Credentials on device");
				prov_data.isProvisioned = 1;
				strcpy(prov_data.ssid, (char *)wifi_configSTA.sta.ssid);
				strcpy(prov_data.password, (char *)wifi_configSTA.sta.password);
				update_provisioned_data();
			}
			break;

		case prov_wrong_cred:
			if(xStateCMD == prov_start_scan)
			{
				xStateCMD = 0;
				xStateNext = prov_start_scan;
				ESP_ERROR_CHECK(esp_wifi_disconnect());
			}
			else if(xEventGroupGetBits(common_events_group) & (1 << enum_wifi_connected))
			{
				xStateNext = prov_connected_to_router;
			}
			else
				xStateNext = prov_wrong_cred;
			break;
		default:
			break;
		}

		xStateCurrent = xStateNext;
		vTaskDelay(pdMS_TO_TICKS(300));
	}

	return ret;
}

char pcState[32];
char pcPostResOk[] = "{\"Ok\":1}";
char pcPostResFail[] = "{\"Ok\":0}";

void vGetPostData(httpd_req_t *req)
{
	int iLength ;
	iLength = (req->content_len < CPOSTBUFSIZE) ? req->content_len : CPOSTBUFSIZE-1;
	memset(cPostReqBuf, 0 ,sizeof(cPostReqBuf));
	httpd_req_recv(req, cPostReqBuf,iLength);
}

esp_err_t xGetProvState(httpd_req_t *req)
{
	sprintf(pcState, "{\"state\":%d}", xStateCurrent);
	httpd_resp_send(req, (char *)pcState, strlen(pcState));
	return ESP_OK;
}

httpd_uri_t xGetProvStateUri = {
		.uri       = "/getProvState",
		.method    = HTTP_GET,
		.handler   = xGetProvState,
		.user_ctx  = NULL,
};

esp_err_t xPostProvCMD(httpd_req_t *req)
{
	vGetPostData(req);
	ESP_LOGI(TAG, "[%s]", cPostReqBuf);
	cJSON *root = cJSON_Parse(cPostReqBuf);
	xStateCMD = cJSON_GetObjectItem(root, "cmd")->valueint;
	cJSON_Delete(root);
	httpd_resp_send(req, (char *)pcPostResOk, strlen(pcPostResOk));
	return ESP_OK;
}

httpd_uri_t xPostProvCMDUri = {
		.uri       = "/provCMD",
		.method    = HTTP_POST,
		.handler   = xPostProvCMD,
		.user_ctx  = NULL,
};

esp_err_t xPostWifiSSIDList(httpd_req_t *req)
{
	vGetPostData(req);

	cJSON *root = cJSON_CreateObject();
	cJSON *array= cJSON_CreateArray();
	for(int i = 0; i < u8ApListEntry; i++)
	{
		cJSON *item = cJSON_CreateObject();
		cJSON_AddStringToObject(item, "ssid", (char *)xApList[i].ssid);
		cJSON_AddNumberToObject(item, "security", xApList[i].authmode);
		cJSON_AddItemToArray(array, item);
	}
	cJSON_AddItemToObject(root, "ssidList", array);
	char *res;
	res = cJSON_PrintUnformatted(root);
	httpd_resp_send(req, (char *)res, strlen(res));
	return ESP_OK;
}

httpd_uri_t xGetWifiSSIDListUri = {
		.uri       = "/scanlist",
		.method    = HTTP_GET,
		.handler   = xPostWifiSSIDList,
		.user_ctx  = NULL,
};

esp_err_t xGetRootESP(httpd_req_t *req)
{
	httpd_resp_send(req, (char *)pcPostResOk, strlen(pcPostResOk));
	ESP_LOGI(TAG, " *** Rebooting device...");
	vTaskDelay(pdMS_TO_TICKS(3000));
	esp_restart();
	return ESP_OK;
}

httpd_uri_t xGetRootESPUri = {
		.uri       = "/reboot",
		.method    = HTTP_GET,
		.handler   = xGetRootESP,
		.user_ctx  = NULL,
};

esp_err_t xPostWifiCred(httpd_req_t *req)
{
	vGetPostData(req);
	cJSON *root = cJSON_Parse(cPostReqBuf);
	ESP_LOGI(TAG, "SSID = [%s]", cJSON_GetObjectItem(root, "ssid")->valuestring);
	ESP_LOGI(TAG, "PASS = [%s]", cJSON_GetObjectItem(root, "pass")->valuestring);
	strcpy((char *)wifi_configSTA.sta.ssid, cJSON_GetObjectItem(root, "ssid")->valuestring);
	strcpy((char *)wifi_configSTA.sta.password, cJSON_GetObjectItem(root, "pass")->valuestring);

	cJSON_Delete(root);
	httpd_resp_send(req, (char *)pcPostResOk, strlen(pcPostResOk));

	if(xStateCurrent == prov_wrong_cred)
	{
		ESP_ERROR_CHECK(esp_wifi_disconnect());
	}

	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configSTA));
	ESP_ERROR_CHECK(esp_wifi_connect());

	ESP_LOGI(TAG, "Connecting to %s network", wifi_configSTA.sta.ssid);
	xEventGroupSetBits(common_events_group, (1 << enum_wifi_connecting));
	xEventGroupClearBits(common_events_group, (1 << enum_wifi_connecting));
	xEventGroupClearBits(common_events_group, (1 << enum_wifi_connected));


	return ESP_OK;

}

httpd_uri_t xPostWifiCredUri = {
		.uri       = "/wificred",
		.method    = HTTP_POST,
		.handler   = xPostWifiCred,
		.user_ctx  = NULL,
};

esp_err_t xPostDeviceType(httpd_req_t *req)
{
	uint8_t success = 0;
	uint8_t device = 0;
	vGetPostData(req);
	cJSON *root = cJSON_Parse(cPostReqBuf);
	if(root)
	{

		if(cJSON_HasObjectItem(root, key_w_device_type))
		{
			device = cJSON_GetObjectItem(root, key_w_device_type)->valueint;
			if( (device == ENUM_DEVICE_TYPE_PFC) || (device == ENUM_DEVICE_TYPE_RS485))
			{
				device_type = device;
				update_device_type();
				success = 1;
			}
		}
		cJSON_Delete(root);

	}

	if(success)
		httpd_resp_send(req, (char *)pcPostResOk, strlen(pcPostResOk));
	else
		httpd_resp_send(req, (char *)pcPostResFail, strlen(pcPostResFail));

	return ESP_OK;

}

httpd_uri_t xPostDeviceTypeUri = {
		.uri       = "/device",
		.method    = HTTP_POST,
		.handler   = xPostDeviceType,
		.user_ctx  = NULL,
};

esp_err_t xGetDeviceInfo(httpd_req_t *req)
{
	uint8_t deviceETH[6];
	char mac_str[32];
	esp_wifi_get_mac(WIFI_IF_STA, deviceETH);
	sprintf(mac_str, "%02X%02X%02X%02X%02X%02X",
			deviceETH[0], deviceETH[1], deviceETH[2],
			deviceETH[3], deviceETH[4], deviceETH[5]);

	ESP_LOGI(TAG, "Device Info : ");
	cJSON *root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "device_type", device_type);
	cJSON_AddStringToObject(root, "mac", mac_str);
	char *res;
	res = cJSON_PrintUnformatted(root);
	httpd_resp_send(req, (char *)res, strlen(res));
	cJSON_Delete(root);
	return ESP_OK;
}

httpd_uri_t xGetInfoUri = {
		.uri       = "/info",
		.method    = HTTP_GET,
		.handler   = xGetDeviceInfo,
		.user_ctx  = NULL,
};

esp_err_t xPostRS485Param(httpd_req_t *req)
{
	uint8_t success = 0;
	uint8_t slave_id = 1;
	uint32_t baud_rate = 9600;

	vGetPostData(req);
	cJSON *root = cJSON_Parse(cPostReqBuf);
	if(root)
	{

		if(cJSON_HasObjectItem(root, key_w_baud_rate) && cJSON_HasObjectItem(root, key_w_slave_id))
		{
			baud_rate = cJSON_GetObjectItem(root, key_w_baud_rate)->valueint;
			slave_id = cJSON_GetObjectItem(root, key_w_slave_id)->valueint;
			mbus_comm_parameter.baud_rate = baud_rate;
			mbus_comm_parameter.slave_id = slave_id;
			save_mbus_comm_param();
			success = 1;
		}
		cJSON_Delete(root);

	}

	if(success)
		httpd_resp_send(req, (char *)pcPostResOk, strlen(pcPostResOk));
	else
		httpd_resp_send(req, (char *)pcPostResFail, strlen(pcPostResFail));

	return ESP_OK;

}

httpd_uri_t xPostrs485ParamUri = {
		.uri       = "/rs485param",
		.method    = HTTP_POST,
		.handler   = xPostRS485Param,
		.user_ctx  = NULL,
};

esp_err_t xGetrs485ParamInfo(httpd_req_t *req)
{

	ESP_LOGI(TAG, "Modbus parameters");
	cJSON *root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, key_w_baud_rate, mbus_comm_parameter.baud_rate);
	cJSON_AddNumberToObject(root, key_w_slave_id, mbus_comm_parameter.slave_id);
	char *res;
	res = cJSON_PrintUnformatted(root);
	httpd_resp_send(req, (char *)res, strlen(res));
	cJSON_Delete(root);
	return ESP_OK;
}

httpd_uri_t xGetrs485ParamInfoUri = {
		.uri       = "/getrs485param",
		.method    = HTTP_GET,
		.handler   = xGetrs485ParamInfo,
		.user_ctx  = NULL,
};

esp_err_t xPostgetmbusmap(httpd_req_t *req)
{
	uint8_t success = 0;

	uint8_t id = 0;

	vGetPostData(req);
	cJSON *rcvd = cJSON_Parse(cPostReqBuf);
	if(rcvd)
	{

		if(cJSON_HasObjectItem(rcvd, key_w_id))
		{
			id = cJSON_GetObjectItem(rcvd, key_w_id)->valueint;
			if(id < MAX_MBUS_REG)
				success = 1;
		}
		cJSON_Delete(rcvd);

	}

	if(success)
	{
		cJSON *response = cJSON_CreateObject();
		cJSON_AddNumberToObject(response, key_w_id, id);
		cJSON_AddNumberToObject(response, key_w_address, mbus_map_element[id].addr);
		cJSON_AddNumberToObject(response, key_w_enable, mbus_map_element[id].enable);
		cJSON_AddNumberToObject(response, key_w_param, mbus_map_element[id].parameter);
		char *res;
		res = cJSON_PrintUnformatted(response);
		httpd_resp_send(req, (char *)res, strlen(res));
		cJSON_Delete(response);
		return ESP_OK;
	}

	else
		httpd_resp_send(req, (char *)pcPostResFail, strlen(pcPostResFail));

	return ESP_OK;

}

httpd_uri_t xPostgetmbusmapUri = {
		.uri       = "/getmbusmap",
		.method    = HTTP_POST,
		.handler   = xPostgetmbusmap,
		.user_ctx  = NULL,
};

esp_err_t xPostsetmbusmap(httpd_req_t *req)
{
	uint8_t success = 0;

	uint8_t id = 0;
	uint8_t enable = 0;
	uint16_t addr = 1;
	uint16_t param = 0;

	vGetPostData(req);
	cJSON *rcvd = cJSON_Parse(cPostReqBuf);
	if(rcvd)
	{

		if(cJSON_HasObjectItem(rcvd, key_w_id) &&
				cJSON_HasObjectItem(rcvd, key_w_address) &&
				cJSON_HasObjectItem(rcvd, key_w_param) &&
				cJSON_HasObjectItem(rcvd, key_w_enable)

		)
		{
			id = cJSON_GetObjectItem(rcvd, key_w_id)->valueint;
			enable = cJSON_GetObjectItem(rcvd, key_w_enable)->valueint;
			addr = cJSON_GetObjectItem(rcvd, key_w_address)->valueint;
			param = cJSON_GetObjectItem(rcvd, key_w_param)->valueint;

			if(id < MAX_MBUS_REG)
				success = 1;
		}
		cJSON_Delete(rcvd);

	}

	if(success)
	{
		mbus_map_element[id].addr = addr;
		mbus_map_element[id].enable = enable;
		mbus_map_element[id].parameter = param;
		save_mbus_map();
		httpd_resp_send(req, (char *)pcPostResOk, strlen(pcPostResOk));
	}
	else
		httpd_resp_send(req, (char *)pcPostResFail, strlen(pcPostResFail));

	return ESP_OK;

}

httpd_uri_t xPostsetmbusmapUri = {
		.uri       = "/setmbusmap",
		.method    = HTTP_POST,
		.handler   = xPostsetmbusmap,
		.user_ctx  = NULL,
};


esp_err_t xPostEndpoint(httpd_req_t *req)
{
	char data[CPOSTBUFSIZE];
	int status = 0;
	vGetPostData(req);
	cJSON *root = cJSON_Parse(cPostReqBuf);
	if(root)
	{
		if(cJSON_HasObjectItem(root, "ep"))
		{
			status = 1;
			strcpy(data, cJSON_GetObjectItem(root, "ep")->valuestring);
			ESP_LOGI(TAG, "EP : %s ", data);
			save_endpoint(data, strlen(data));

		}
		cJSON_Delete(root);
		if(status ==1)
		{
			httpd_resp_send(req, (char *)pcPostResOk, strlen(pcPostResOk));
		}
		else
		{
			httpd_resp_send(req, (char *)pcPostResFail, strlen(pcPostResFail));
		}
	}
	else
	{
		httpd_resp_send(req, (char *)pcPostResFail, strlen(pcPostResFail));
	}

	return ESP_OK;
}

httpd_uri_t xPostEndpointUri = {
		.uri       = "/endpoint",
		.method    = HTTP_POST,
		.handler   = xPostEndpoint,
		.user_ctx  = NULL,
};

void start_provisioning_server()
{
	if(xServer != NULL)
		return;

	ESP_LOGI(TAG, "Stared Server for provisioning");

	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.max_uri_handlers = 15;
	if(httpd_start(&xServer, &config) == ESP_OK)
	{
		httpd_register_uri_handler(xServer, &xGetProvStateUri);
		httpd_register_uri_handler(xServer, &xPostProvCMDUri);
		httpd_register_uri_handler(xServer, &xGetWifiSSIDListUri);
		httpd_register_uri_handler(xServer, &xGetRootESPUri);
		httpd_register_uri_handler(xServer, &xPostWifiCredUri);
		httpd_register_uri_handler(xServer, &xGetInfoUri);
		httpd_register_uri_handler(xServer, &xPostDeviceTypeUri);
		httpd_register_uri_handler(xServer, &xPostrs485ParamUri);
		httpd_register_uri_handler(xServer, &xGetrs485ParamInfoUri);
		httpd_register_uri_handler(xServer, &xPostgetmbusmapUri);
		httpd_register_uri_handler(xServer, &xPostsetmbusmapUri);
		httpd_register_uri_handler(xServer, &xPostEndpointUri);
	}
}

esp_err_t get_rssi_info(uint8_t *rssi)
{
	wifi_ap_record_t wifidata;
	if (esp_wifi_sta_get_ap_info(&wifidata)==ESP_OK)
	{
		ESP_LOGI(TAG, "RSSI : %d \r\n", wifidata.rssi);
		*rssi = wifidata.rssi;
		return ESP_OK;
	}
	else
		return ESP_FAIL;
}

