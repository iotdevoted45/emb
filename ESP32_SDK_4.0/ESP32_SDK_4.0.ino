 /*
  SDK for IoTConnect

  This IoTConnect SDK will help you to update your Sensors data on IoTConnect cloud(Azure)
  In this example file Humidity, Temperature and vibration(x,y,z) random data published on our cloud at real time

  For run this example you have to include/import "IoTConnect.cpp" or "IoTConnect.h"
  you will need wifi connection for publishing data to IoTCOnnect ..
  For the secure MQTT connection here we are using "SAS Token key"

  for more help and informationvisit https://help.iotconnect.io SDK section

    modified: 15March2021
*/

/*
  Hope you have installed the node SDK as guided on SDK documentation.
*/

#include "config/IOT_config.h"
#include "client/IoTConnect.cpp"

//making IOTConnectClient client for use IOTConnectClient functions  
IOTConnectClient SDKClient;      

unsigned long lastMillis = 0;
int count = 0;

String Months_inNumber(String in) {
  if (in == "Jan")  return "01";  else if (in == "Feb")  return "02";
  else if (in == "Mar")  return "03";  else if (in == "Apr")  return "04";
  else if (in == "May")  return "05";  else if (in == "Jun")  return "06";
  else if (in == "Jul")  return "07";  else if (in == "Aug")  return "08";
  else if (in == "Sep")  return "09";  else if (in == "Oct")  return "10";
  else if (in == "Nov")  return "11";  else if (in == "Dec")  return "12";
}

// Get time From NTP and setup in SDK format
String GetTime() {             // Setup UCT time from NTP server
  String Year, Month, Day1, Day2, Time;
  time_t now = time(nullptr);
  String time_now = (ctime(&now));
  //Serial.println(T);
  if (time_now.substring(20, 22) == "20") {
    Day1 = time_now.substring(8, 9);          if (Day1 == " ")Day1 = "0";
    Day2 = time_now.substring(9, 10);
    Month = Months_inNumber(time_now.substring(4, 7));   Year = time_now.substring(20, 24);
    Time = time_now.substring(11, 19);
    return (Year + "-" + Month + "-" + Day1 + Day2 + "T" + Time + ".000Z");
  }
  return (Year + "-" + Month + "-" + Day1 + Day2 + "T" + Time + ".000Z");
}

void setModuleRecievedCallback(String PayLoad){

/* Here we will get device Module payload from cloud to device */
Serial.println("\r\n *** Module Recieved Payload : " + PayLoad);
}

void setDeviceChangedCallback(String PayLoad){

/* Here we will get Device change payload from cloud to device */
Serial.println("\r\n *** Device Changed Payload : " + PayLoad);
}

/*
  Type    : Callback Function "setTwinChangedCallback()"
  Usage   : Manage twin properties as per business logic to update the twin reported property
  Output  : Receive twin properties Desired, Reported
  Input   :
*/
// String Key = " Desired property key "; // Desired proeprty key received from Twin callback message
// String Value = " Desired Property value "; // Value of respective desired property
String Key = "", Value = "";
void setTwinChangedCallback(String &PayLoad) {

  Serial.println("\r\n*** Received Twin with Data:" + PayLoad);
  StaticJsonDocument<512> in_cmds;
  deserializeJson(in_cmds, PayLoad);
  JsonObject in_cmd = in_cmds.as<JsonObject>();
  JsonObject desired = in_cmd["desired"].as<JsonObject>();
  for (JsonPair keyValue : desired) {
      Key = keyValue.key().c_str();
      if(Key == "$version"){
        
      }else{
        Value = in_cmd["desired"][Key].as<String>();
        
/*
Type    : Public Method "updateTwin()"
Usage   : Upate the twin reported property
Output  :
Input   : "key" and "value" as below
*/
 //     SDKClient.UpdateTwin(Key,Value);
        }      
  }
}


/*
  Type    : Callback Function "CallbackMessage()"
  Usage   : Firmware will receive commands from cloud. You can manage your business logic as per received command.
  Output  : Receive device command, firmware command and other device initialize error response
  Input   :
*/
void setConnectionStatusChangedCallback(String PayLoad) {

  String cmd_ackID = "";   String Cmd_value = "";
  StaticJsonDocument<512> in_cmd;
  deserializeJson(in_cmd, PayLoad);
  cmd_ackID = in_cmd["ackId"].as<String>();
  Cmd_value = in_cmd["cmdType"].as<String>();

  if (Cmd_value == "0x16" ) {
    int Cmd = in_cmd["command"].as<int>();
    if (Cmd == 1)
      Serial.println("\r\n\t ** Device Connected ** \r\n");
    if (Cmd == 0)
      Serial.println("\r\n\t ** Device Disconnected ** \r\n");
    return ;
  }

}

void setDeviceCommandCallback(String &PayLoad) {

  Serial.println("\r\n*** Received Command with Data:" + PayLoad);
  int success = 1;
  StaticJsonDocument<2048> in_cmd;
  deserializeJson(in_cmd, PayLoad);
  String cmd_ackID = in_cmd["ackId"].as<String>();

  DynamicJsonDocument  ACK_JSON(1024);
  ACK_JSON["ackId"] = cmd_ackID;
  ACK_JSON["msg"] = "Device command received successfully";
  ACK_JSON["childId"] = "";
  String Ack_Json_Data;
  serializeJson(ACK_JSON, Ack_Json_Data);

   SDKClient.sendAckCmd(success, Ack_Json_Data);
}

void setOTAReceivedCallback(String &PayLoad) {

  Serial.println("\r\n*** Received OTA with Data:" + PayLoad);
  int success = 1;
  StaticJsonDocument<2048> in_cmd;
  deserializeJson(in_cmd, PayLoad);
  String cmd_ackID = in_cmd["ackId"].as<String>();

  DynamicJsonDocument  ACK_JSON(1024);
  ACK_JSON["ackId"] = cmd_ackID;
  ACK_JSON["msg"] = "OTA updated successfully";
  ACK_JSON["childId"] = "";
  String Ack_Json_Data;
  serializeJson(ACK_JSON, Ack_Json_Data);

   SDKClient.sendAckOta(success, Ack_Json_Data);
}


void setup() {

  int err = 0;
  delay(50);
  Serial.begin(115200);                // setup Serial Monitor
  WiFi.mode(WIFI_STA);    WiFi.begin(WiFi_SSID, WiFi_PASS);
  Serial.println("\n\tConnecting WiFi ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
    Serial.print("- ");
  }
  Serial.println("\n\tWiFi Connected");
  Serial.print("\tDevice_IP: "); Serial.println(WiFi.localIP());
  configTime(0, 0, "pool.ntp.org");           // getting UCT time from NTP server
  // (timezone*3600,dst,"pool.ntp.org","time.nist.gov");

  DynamicJsonDocument  sdkOptions(1024);
  sdkOptions["discoveryUrl"] = "discovery.iotconnect.io"; //Discovery URL is mandatory parameter to get device details
  sdkOptions["domainFingerprint"] = "";                            // If you are using ESp32 device no need for that
  sdkOptions["IsDebug"] = "true";
  String SdkOptions;
  serializeJson(sdkOptions, SdkOptions);
  /*
    Initialize SDK. you have to pass IOTC_cpId, IOTC_deviceId, deviceCallback, twinUpdateCallback, IOTC_env All variables are defined above.

    Type    : Object Initialization "new SDKClient()"
    Usage   : To Initialize SDK and Device cinnection
    Output  : Callback methods for command and twin properties
    Input   : IOTC_cpId, IOTC_deviceId, env as explained above
  */
  err = SDKClient.Init(IOTC_SID, IOTC_DeviceId, SdkOptions);
  if (err) {
    Serial.println("ERROR while Init 'IOTCONNECT'");
  }

  delay(30);
  SDKClient.Connect(setDeviceCommandCallback, setOTAReceivedCallback, setTwinChangedCallback);

  /*
    Type    : Public Method "getAllTwins()"
    Usage   : To get all the twin properies Desired and Reported
    Output  : All twin property will receive in above callback function "twinUpdateCallback()"
  */

  SDKClient.GetAllTwins();
  delay(3000);
}

void loop() {
  SDKClient.MQTT_loop();           // SDK connection in infinite loop
      delay(50);
      //////////////////////////////////////////////////////////////////////////////////
      DynamicJsonDocument  Attribute_json(2048);                                      //
          JsonArray Device_data = Attribute_json.to<JsonArray>();                     //   Send data input json format
          JsonObject Device_01 = Device_data.createNestedObject();                    //
          Device_01["uniqueId"] = IOTC_DeviceId;                                           //
          Device_01["time"] = GetTime();                                              //
          JsonObject D01_Sensors=Device_01.createNestedObject("data");                //
    //          D01_Sensors["Humidity"] = random(20, 80);                               //
              D01_Sensors["temp"] = random(12, 32);
//          JsonObject Device_02 = Device_data.createNestedObject();                    //
//          Device_02["uniqueId"] = "ACMAY07";                                           //
//          Device_02["time"] = GetTime();                                              //
//          JsonObject D02_Sensors=Device_02.createNestedObject("data");                //
//    //          D01_Sensors["Humidity"] = random(20, 80);                               //
//              D02_Sensors["temp"] = random(12, 32);//
//          JsonObject Device_03 = Device_data.createNestedObject();                    //
//          Device_03["uniqueId"] = "rtM07";                                           //
//          Device_03["time"] = GetTime();                                              //
//          JsonObject D03_Sensors=Device_03.createNestedObject("data");                //
//    //          D01_Sensors["Humidity"] = random(20, 80);                               //
//              D03_Sensors["temp"] = random(42, 77); ;//
//  //        JsonObject D01_SensorsV = D01_Sensors.createNestedObject("vibration");      //
//  //            D01_SensorsV["x"] = random(12,  50);                                    //
//  //            D01_SensorsV["y"] = random(-20, 30);                                    //
//  //            D01_SensorsV["z"] = random(10,  35);                                    //
      String Attribute_json_Data;

      serializeJson(Attribute_json, Attribute_json_Data);                             //
      //////////////////////////////////////////////////////////////////////////////////
  // publish a message roughly every 10000 millisecond.
      if (millis() - lastMillis > 10000) {
          lastMillis = millis();
  /*
  Type    : Public Method "sendData()"
  Usage   : To publish the D2C data
  Output  :
  Input   : Predefined data object
  */
         SDKClient.SendData(Attribute_json_Data);
          count = count + 1;
      }
      Attribute_json.clear();
  
      if (count > 200){
  /*
  Type    : Public Method "dispose()"
  Usage   : Disconnect the device from cloud
  Output  :
  Input   :
  Note : It will disconnect the device after defined time
  */
        SDKClient.Dispose();
  
        }
  
}
