
# IOTConnect SDK: iotconnect-sdk-c-nRF9160

This is an C Lang SDK library for "nRF9160" board to connect the device with IoTConnect cloud by MQTT protocol.
This library only abstract JSON responses from both end D2C and C2D. This SDK supports x509 based authentication only by using secure certificates.

## Features:

* The SDK supports to send telemetry data and receive commands from IoTConnect portal.
* User can update firmware Over The Air using "OTA update" Feature supported by SDK.
* SDK supports to receive and update the Twin property. 
* SDK supports device and OTA command Acknowledgement device.
* Provide device connection status receive by command.
* Support hard stop command to stop device client from cloud.
* It manages the sensor data sending flow over the cloud by using data frequency("df") configuration.
* It allows to disconnect the device from firmware.

# Example Usage:

Import library
```c++
#include "IoTConnect_Config.h"
#include "main.h"
```
- Prerequisite standard configuration data
```c++
#define IOTCONNECT_DEVICE_UNIQUE_ID    "from IoTConnect portal"
#define IOTCONNECT_DEVICE_CP_ID        "from IoTConnect portal"
#define IOTCONNECT_DEVICE_ENV          "from IoTConnect portal"
```
"IOTCONNECT_DEVICE_UNIQUE_ID" : Its device ID which register on IotConnect platform and also its status has Active and Acquired.
"IOTCONNECT_DEVICE_CP_ID" : It is the company code. It gets from the IoTConnect UI portal "Settings->Key Vault".
"IOTCONNECT_DEVICE_ENV" : It is the UI platform environment. It gets from the IoTConnect UI portal "Settings->Key Vault".

- SdkOptions is for the SDK configuration and needs to parse in SDK object initialize call. You need to manage the below configuration as per your device authentication type.
```c++
cJSON *SDKoption;
SDKoption = cJSON_CreateObject();
char *SDK_option = cJSON_PrintUnformatted(SDKoption);
```

- To Initialize the SDK object and connect to the cloud
```c++
IoTConnect_init(IOTCONNECT_DEVICE_CP_ID, IOTCONNECT_DEVICE_UNIQUE_ID, DeviceCallback, TwinUpdateCallback, SDK_option, IOTCONNECT_DEVICE_ENV);
``` 

- To Connect mqtt client with IoTConnect cloud 
```c++
IoTConnect_connect();
```

- This is the standard data input format for non Gateway device to send the data on IoTConnect cloud(D2C).
```c++
// For Non Gateway Device 
String Attribute_json_Data = [{
	"uniqueId": "<< Device UniqueId >>",
	"time" : "<< date >>",
	"data": {}
}];
SendData(Attribute_json_Data);
```
"time" : Date format should be as defined # "2021-01-24T10:06:17.857Z"
"data" : JSON data type format # {"temperature": 15.55, "gyroscope" : { 'x' : -1.2 }}
	
- To receive the command from Cloud to Device(C2D) and to send the command acknowledgment from device to cloud.
```c++
void DeviceCallback(char *payload){
	printk("\n Cmd_msg >>  %s",payload);
	if(payload.cmdType == "0x01")
		// Device Command
	if(payload.cmdType == "0x02")
		// Firmware Command
	if(payload.cmdType == "0x16")
		// Device connection status
}   
```

- To send the command acknowledgment from device to cloud.
```c++
	Ack_Json=cJSON_CreateObject();
	cJSON_AddStringToObject(Ack_Json, "ackId", cmd_ackID);
	cJSON_AddStringToObject(Ack_Json, "msg", "");
	cJSON_AddStringToObject(Ack_Json, "childId", cmd_UNI);
	cJSON_AddNumberToObject(Ack_Json, "st", Status);
	char *Ack_Json_Data = cJSON_Print(Ack_Json);

    int msgType = ""; // 5 ("0x01" device command), 11 ("0x02" Firmware OTA command)
    SendAck(Ack_Json_Data, msgType);
```
"ackId(*)" 	: Command Acknowledgment GUID which will receive from command payload (data.ackId)
"st(*)"		: Acknowledgment status sent to cloud (4 = Fail, 6 = Device command[0x01], 7 = Firmware OTA command[0x02])
"msg" 		: It is used to send your custom message
"childId" 	: It is used for Gateway's child device OTA update only
				0x01 : null or "" for Device command
			  	0x02 : null or "" for Gateway device and mandatory for Gateway child device's OTA update.
		   		How to get the "childId" .?
		   		- You will get child uniqueId for child device OTA command from payload "data.urls[~].uniqueId"
"msgType" 	: Message type (5 = "0x01" device command, 11 = "0x02" Firmware OTA command)
Note : (*) indicates the mandatory element of the object.


- To receive the twin from Cloud to Device(C2D).
```c++
void TwinUpdateCallback(char *payload){
	/* Here we will get device twin properties payload from cloud to device */
	printk("\n Twin_msg payload is >>  %s", payload);
} 
```

-To receive the All twins from Cloud to Device(C2D)
```c++
char *key = "twin01", *value = "18";
// String key = "<< Desired property key >>";
// String value = "<< Desired Property value >>";
UpdateTwin(key,value);
```
"key" : Desired property key received from Twin callback message
"value" : Value of the respective desired property

-To Stopped the IoTConnect SDK
```c++
IoTConnect_abort();   
```

-To receive the All twins from Cloud to Device(C2D)
```c++
GetAllTwins();
```

# Dependencies:
* This SDK used below packages :
	-This SDK based on zephyr and nefxlib packages with nRF Connect

# Integration Notes:

## Prerequisite tools

1.	Install nRF9160 SDK with 1.2.0 version and complete Getting Started with nRF9160 DK.
	- Reference link to install (https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/index.html)
2.  Install the SEGGER Embedded Studio with 4.50 and nRF connect 3.6.0 version for edit and run the application 

## Installation:
1. 	For making new application, create a folder in nrf-sample folder and name is "IoTConnect" where you installed nRFConnet SDK.
	- Path to create folder : (...Nordic\ncs\nrf\samples\nrf9160\IoTConnect).
2. 	Now unzip the "iotconnect-sdk-c-nRF9160-v3.0.zip" SDK which you can download from our IoTConnect help portal.
3.	We have "IoTConnect_config.h" file in "nRF9160-DK\src".
	- you need to input "uniqueIdID", "CPID" and "env" in IoTConnect_config.h file. You can see more in below section title with "Prerequisite input data"
4.	In the other folder "nRF9160-DK\IoTConnect\cert" we have a "certificate.h" in here you have to put your device certificate 

```c++
#define CLOUD_CLIENT_PRIVATE_KEY \
"-----BEGIN RSA PRIVATE KEY-----\n"
----------------------------------
----------------------------------
"-----END RSA PRIVATE KEY-----\n"

#define CLOUD_CLIENT_PUBLIC_CERTIFICATE \
"-----BEGIN CERTIFICATE-----\n"
----------------------------------
----------------------------------
"-----END CERTIFICATE-----\n"

#define CLOUD_CA_CERTIFICATE \
"-----BEGIN CERTIFICATE-----\n" \
----------------------------------
----------------------------------
"-----END CERTIFICATE-----\n"
```

## Release Note :

1.  Device is using "X.509" certificates for MQTT connection with IoTConnect
2.  Support Device and OTA command with acknowledgment
3.  Support hard stop command to stop device client from cloud
4.  It allows sending the OTA command acknowledgment
5.  User can get device all twin properties with update Desired properties 
6.  IoTConnect_abort() allows to disconnect the device client

** New Feature **
1. Introduce "SDKoption" initialize the SDK.
2. Introduce new command(0x16) in device callback for Device connection status true(connected) or false(disconnected)
3. Use the "df" Data Frequency feature to control the flow of data which publish on cloud. 

** Improvements **
1. We have updated below methods name:
   To Initialize the SDK object:
	- Old : IoTConnect_init(IOTCONNECT_DEVICE_CP_ID, IOTCONNECT_DEVICE_UNIQUE_ID, IOTCONNECT_DEVICE_ENV, Device_CallBack, Twin_CallBack);
	- New : IoTConnect_init(IOTCONNECT_DEVICE_CP_ID, IOTCONNECT_DEVICE_UNIQUE_ID, DeviceCallback, TwinUpdateCallback, SDK_option, IOTCONNECT_DEVICE_ENV);
    To receive Device command callback :
    - Old : Device_CallBack(char *topic, char *payload);
	- New : DeviceCallback(char *payload);
   To receive twin command callback :
    - Old : Twin_CallBack(char *topic, char *payload) ;
	- New : TwinUpdateCallback(char *payload);
3. Update the OTA command receiver payload for multiple OTA files
