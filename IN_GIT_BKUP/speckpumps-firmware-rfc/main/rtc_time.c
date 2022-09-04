/**************************
*  @developer          	> Steve_Shubhash
*  @last_updated_by    	> Shane_Shekhar
*  @FW_version         	> 0.2.2
*  @date               	> 02SEPT2022
**************************/


#include "rtc_time.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

#include <time.h>
#include <sys/time.h>
#include "esp_sntp.h"

#include "fileOperations.h"
#include "i2c_peri.h"

static void initialize_sntp(void);
static void time_sync_notification_cb(struct timeval *tv);
struct tm get_local_time();
esp_err_t get_internet_time();

void task_time_sync(void *pv)
{
	ESP_LOGI(TAG, "Starting time sync task");
	initialize_sntp();

	struct tm rtc_tm;
	if(MCP79410_RTC_getTime(&rtc_tm) == ESP_OK)
	{
		ESP_LOGI("-->", "YEAR = %d", rtc_tm.tm_year);
		if(rtc_tm.tm_year > 100)
		{
			time_t t = mktime(&rtc_tm);
			struct timeval now_tv = { .tv_sec = t };
			settimeofday(&now_tv, NULL);
			read_time_zone();
			ESP_LOGI(TAG, "Device timezone = [%s]", device_time_zone.time_zone);
			set_timezone(device_time_zone.time_zone);
			xEventGroupSetBits(common_events_group, 1 << enum_time_synced);
			get_local_time();
		}
	}
	else
	{
		ESP_LOGE("--->", "Device timezone = [%s]", device_time_zone.time_zone);
		set_timezone(device_time_zone.time_zone);
	}

	while(1)
	{
		if(get_internet_time() == ESP_OK)
		{

			sntp_stop();
			sntp_enabled();
			sntp_init();
			vTaskDelay(pdMS_TO_TICKS((1000)*(60)*(60)*(1))); //Wait for 1 hour
		}
		else
		{

			vTaskDelay(pdMS_TO_TICKS((1000)*(60)*(1)*(1))); //Wait for 1 minutes
		}
	}
}

void set_timezone(char *timezone)
{
	if(strlen(timezone) > 0)
	{
		setenv("TZ", timezone, 1);
		tzset();
	}
}

static void initialize_sntp(void)
{

	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, "time.google.com");
	sntp_setservername(1, "pool.ntp.org");

	read_time_zone();
	//set_timezone(device_time_zone.time_zone);

	ESP_LOGI(TAG,"Timezone: [%s]", device_time_zone.time_zone);
	ESP_LOGI(TAG, "Initializing SNTP ... ");

	sntp_set_time_sync_notification_cb(time_sync_notification_cb);
	sntp_init();
}

void time_sync_notification_cb(struct timeval *tv)
{
	xEventGroupSetBits(common_events_group, 1 << enum_time_synced);
	ESP_LOGI(TAG,"Time is synced");

	get_local_time();

	time_t currentUnixTime;
	time(&currentUnixTime);
	struct tm currentTM = *gmtime(&currentUnixTime);
	if(MCP79410_RTC_setTime(&currentTM) != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to read time");
	}
	else
		ESP_LOGI("--->", "Updated time in RTC");

}

struct tm get_local_time()
{
	time_t now;
	struct tm timeinfo;
	time(&now);

	char strftime_buf[64];

	localtime_r(&now, &timeinfo);
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
	ESP_LOGI(TAG, "The current date/time: %s", strftime_buf);
	return timeinfo;
}

esp_err_t get_internet_time()
{
	
	xEventGroupWaitBits(common_events_group, 1 << enum_connected_to_internet, false, true, portMAX_DELAY);
	ESP_LOGI(TAG, "WIFI network is available");

	time_t now = 0;
	struct tm timeinfo = { 0 };
	int retry = 0;
	int retry_count = 10;

	while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count)
	{
		ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
		time(&now);
		localtime_r(&now, &timeinfo);
		vTaskDelay(6000 / portTICK_PERIOD_MS);
	}

	if(retry >= retry_count)
	{
		return ESP_FAIL;
	}

	return ESP_OK;
}

