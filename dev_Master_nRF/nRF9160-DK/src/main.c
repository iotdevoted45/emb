/**
  ******************************************************************************
  * @file   : main.c
  * @author : Softweb Solutions An Avnet Company
  * @modify : 16-Apr-2021
  * @brief  : Firmware part for C_nRF9160 v3.0
  *
  * This IoTConnect SDK will help you to update your Sensors data on IoTConnect cloud(Azure)
  * In this example file Humidity, Temperature and Gyroscope(x,y,z) random data published on our cloud at real time
  * 
  * For run this example you have to include/import "IoTConnect.cpp" or "IoTConnect.h"
  * you will need wifi connection for publishing data to IoTCOnnect.
  * For the secure MQTT connection here we are using "X.509 certificates" 
  * 
  * for more help and information visit https://help.iotconnect.io SDK section
  * ******************************************************************************
*/

/*	Hope you have installed the node SDK as guided on SDK documentation. */

#include "IoTConnect_Config.h"
#include "main.h"

/* Initialize AT communications */
int at_comms_init(void)
{
	int err;
	err = at_cmd_init();
	if (err) 
	{
		printk("Failed to initialize AT commands, err %d\n", err);
		return err;
	}
	err = at_notif_init();
	if (err) 
	{
		printk("Failed to initialize AT notifications, err %d\n", err);
		return err;
	}
	return 0;
}

/*! Object  - application main function
 *  \param  - None
 *  \return - None
 */
void main(void){  
    int err, count=0;  
    err = bsdlib_init();
    if (err) 
	{
		printk("Failed to initialize bsdlib!");
		return ;
	}
    
    err = at_comms_init();
    if (err) 
	{
		return ;
	}

    err = provision_certificates();
    if (err) 
	{
		return ;
	}

    printk("Waiting for network.. ");
    err = lte_lc_init_and_connect();
    if (err) 
	{
		printk("Failed to connect to the LTE network, err %d\n", err);
		return ;
	}
    printk("OK\n");

    cJSON *SDKoption;
    SDKoption = cJSON_CreateObject();
    if (SDKoption == NULL)
	{
        printk("\nUnable to allocate SDKoption Object in SendAck");
        return ;    
    }
    char *SDK_option = cJSON_PrintUnformatted(SDKoption);

	/*
	## Prerequisite params to run this sample code input in IoTConnect_config.h
	- IOTCONNECT_DEVICE_CP_ID      	:: It need to get from the IoTConnect platform. 
	- IOTCONNECT_DEVICE_UNIQUE_ID  	:: Its device ID which register on IotConnect platform and also its status has Active and Acquired
	- IOTCONNECT_DEVICE_ENV        	:: You need to pass respective environment of IoTConnecct platform
	- SDK_option					:: You need to pas JSON as explained 
	*/
    err = IoTConnect_init(IOTCONNECT_DEVICE_CP_ID, IOTCONNECT_DEVICE_UNIQUE_ID, DeviceCallback, TwinUpdateCallback, SDK_option, IOTCONNECT_DEVICE_ENV);
    if (err) 
	{
		printk("Failed to Init IoTConnect SDK");
		return ;
	}

	/*
	Type    : Public Method "IoTConnect_connect()"
	Usage   : To connect with IoTConnect MQTT broker
	*/
	IoTConnect_connect();

	/*
	Type    : Public Method "getAllTwins()"
	Usage   : To get all the twin properties Desired and Reported
	Output  : All twin property will receive in above callback function "TwinUpdateCallback()"
	*/
	GetAllTwins();

	while(count < 15){
		MQTT_looP();
        
		// all sensors data will be formed in JSON format and will be published by SendData() function 
		Attribute_json_Data = Sensor_data();

		/*
		Type    : Public Method "sendData()"
		Usage   : To publish the D2C data 
		Output  : 
		Input   : Predefined data object 
		*/      
		SendData(Attribute_json_Data);
		k_sleep(10000);
		count++ ; 
	}

	/*
	Type    : Public Method "IoTConnect_abort()"
	Usage   : Disconnect the device from cloud
	Output  : 
	Input   : 
	Note : It will disconnect the device after defined time 
	*/ 
     err = IoTConnect_abort();
     if (err) {
        printk("Failed to Abort IoTConnect SDK");
        return ;
     }
     return ;
}

/*
Type    : Callback Function "TwinUpdateCallback()"
Usage   : Manage twin properties as per business logic to update the twin reported property
Output  : Receive twin properties Desired, Reported
Input   : 
*/
void TwinUpdateCallback(char *payload) {      
    char *key = NULL, *value = NULL;
    printk("\n Twin_msg payload is >>  %s", payload);
    
    cJSON *root = cJSON_Parse(payload);        
    cJSON *D = cJSON_GetObjectItem(root, "desired");
    if(D) {
        cJSON *device = D->child;
        while (device){
            if(! strcmp(device->string,"$version")){
       
            }
            else{
                key = device->string;
                value = (cJSON_GetObjectItem(D, key))->valuestring;
               /*
                Type    : Public Method "updateTwin()"
                Usage   : Upate the twin reported property
                Output  : 
                Input   : "key" and "value" as below
                          // String key = "<< Desired property key >>"; // Desired proeprty key received from Twin callback message
                          // String value = "<< Desired Property value >>"; // Value of respective desired property
               */    
                UpdateTwin(key,value);
            }
            device = device->next;
        }		
    }
}

/*
Type    : Callback Function "DeviceCallback()"
Usage   : Firmware will receive commands from cloud. You can manage your business logic as per received command.
Output  : Receive device command, firmware command and other device initialize error response
Input   :  
*/
void DeviceCallback(char *payload) {      
    
    cJSON *Ack_Json, *sub_value, *in_url;
    int Status = 0,magType=0;
    char *cmd_ackID, *Cmd_value, *Ack_Json_Data, *cmd_Uni="";
    printk("\n Cmd_msg >>  %s",payload);   
     
    cJSON *root = cJSON_Parse(payload);
    cmd_ackID = (cJSON_GetObjectItem(root, "ackId"))->valuestring;
    Cmd_value = (cJSON_GetObjectItem(root, "cmdType"))->valuestring;
    if( !strcmp(Cmd_value,"0x16"))
	{
		sub_value = cJSON_GetObjectItem(root,"command");
		int CMD = sub_value->valueint;
		if(CMD == 1){
			printk("\r\n\t ** Device Connected ** \n");
		} 
		else if(CMD == 0) 
		{
			printk("\r\n\t ** Device Disconnected ** \n");
		}
		return;
    }

    if( !strcmp(Cmd_value,"0x01") )
	{
		Status = 6; magType = 5;
	}
    else if( !strcmp(Cmd_value,"0x02") ) 
	{
        Status = 7; magType = 11;
    	sub_value = cJSON_GetObjectItem(root,"urls");
		if(cJSON_IsArray(sub_value)){
            in_url = cJSON_GetArrayItem(sub_value, 0);
            sub_value = cJSON_GetObjectItem(in_url, "uniqueId");
            if(cJSON_IsString(sub_value))
			cmd_Uni = sub_value->valuestring;
		}
    } else { }

    Ack_Json = cJSON_CreateObject();
    if (Ack_Json == NULL)
	{
        printk("\nUnable to allocate Ack_Json Object in Device_CallBack");
        return ;    
    }
    cJSON_AddStringToObject(Ack_Json, "ackId",cmd_ackID);
    cJSON_AddStringToObject(Ack_Json, "msg","");
    cJSON_AddStringToObject(Ack_Json, "childId",cmd_Uni);
    cJSON_AddNumberToObject(Ack_Json, "st", Status);

    Ack_Json_Data = cJSON_PrintUnformatted(Ack_Json);
    
    /*
    Type    : Public Method "sendAck()"
    Usage   : Send firmware command received acknowledgement to cloud
      - status Type
		st = 6; // Device command Ack status 
		st = 7; // firmware OTA command Ack status 
        st = 4; // Failed Ack
      - Message Type
		msgType = 5; // for "0x01" device command 
        msgType = 11; // for "0x02" Firmware command
    */  
    SendAck(Ack_Json_Data, magType);
    cJSON_Delete(Ack_Json);
}

/*! Object  - need to read and setup all sensor(Attribute) data in JSON format
 *  \param  - None
 *  \return - None
 */
char *Sensor_data(void){

    cJSON *Attribute_json = NULL;
    cJSON *Device_data1 = NULL;
    cJSON *Data = NULL, *Data1= NULL;

    Attribute_json = cJSON_CreateArray();
    if (Attribute_json == NULL)
	{
        printk("Unable to allocate Attribute_json Object\n");
        return ;    
    }
    cJSON_AddItemToArray(Attribute_json, Device_data1 = cJSON_CreateObject());
    cJSON_AddStringToObject(Device_data1, "uniqueId",IOTCONNECT_DEVICE_UNIQUE_ID);
    cJSON_AddStringToObject(Device_data1, "time", Get_Time());
    cJSON_AddItemToObject(Device_data1, "data", Data = cJSON_CreateObject());
    cJSON_AddStringToObject(Data,"Humidity", 55 );
    cJSON_AddNumberToObject(Data, "Temperature", 11);
    cJSON_AddItemToObject(Data, "Gyroscope", Data1 = cJSON_CreateObject());
    cJSON_AddNumberToObject(Data1, "x",  128);
    cJSON_AddStringToObject(Data1, "y",  100);
    cJSON_AddNumberToObject(Data1, "z",  318);
	
	/*
	 Non Gateway device input data format Example:
		String data = [{
			"uniqueId": "<< Device UniqueId >>",
			"time" : "<< date >>",
			"data": {}
		}];
	- time : Date format should be as defined # "2021-01-24T10:06:17.857Z"
	- data : JSON data type format # {"temperature": 15.55, "gyroscope" : { 'x' : -1.2 }}
	*/
    char *msg = cJSON_PrintUnformatted(Attribute_json);
    cJSON_Delete(Attribute_json);
    return  msg;
}
