
#include "IOTConnect.h"
#include "../main.h"
WiFiClientSecure Net_MQTT;
 
String G_base_url,G_IoT_SID,G_IoT_DID;
unsigned long int G_Last_DF =0; 

typedef struct Sync_Resp{   
    String device_tg="";
    int at =0;
    int ec =0;               //root..info
    int DF =0;
    String tg="";
    String dvc_ID[10];
    String dvc_TG[10];
    struct protocol{         
        String Name ="";
        String Host ="";
        String ClientID ="";    //data..protocol
        String UserName ="";
        String Password ="";
        String PubTopic ="";
        String SubTopic ="";
        } Broker;
    struct Has{
      int d =0;
      int r =0;
      int att =0;
      int ota =0;
      int sett =0;
      }has;    
};
struct Sync_Resp G_Sync_Resp_info;
IOTConnectClient SDKclient;

static void IOTConnectClientHandler(lwmqtt_client_t * /*client*/, void *ref, lwmqtt_string_t topic, lwmqtt_message_t message) {
    auto cb = (IOTConnectClientCallback *)ref;
    char terminated_topic[topic.len + 1];
    memcpy(terminated_topic, topic.data, topic.len);
    terminated_topic[topic.len] = '\0';
    String str_topic = String(terminated_topic);
    String str_payload;
    if (message.payload != nullptr) {
        message.payload[message.payload_len] = '\0';
    }
    if (cb->advanced != nullptr) {
        cb->advanced(cb->client, terminated_topic, (char *)message.payload, (int)message.payload_len);
        return;
    }
    if (cb->simple_CMD == nullptr) {
      return;
    }

    if (message.payload != nullptr) {
        str_payload = String((const char *)message.payload);
        //Serial.println(str_payload);
    }

    if (str_topic.substring(0, 30) == "$iothub/twin/PATCH/properties/"){
        DynamicJsonDocument root(512);  
        StaticJsonDocument<300> Twin_Json_in;
        deserializeJson(Twin_Json_in, str_payload);
        root["uniqueId"] = G_IoT_DID;
        root["desired"] = Twin_Json_in;
        str_payload = " ";
        serializeJson(root, str_payload);    
        cb->simple_TWN(str_payload);
    }
    else if(str_topic.substring(0, 17) == "$iothub/twin/res/"){
        cb->simple_TWN(str_payload);
    }
    else{
        StaticJsonDocument<2048> incoming;  
        deserializeJson(incoming, str_payload);
        if( (incoming["cmdType"] == "0x01") || (incoming["cmdType"] == "0x02") || (incoming["cmdType"] == "0x03")){
            if(incoming["cmdType"] == "0x01"){
                Serial.println(" INFO_CM01 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Command 0x01 : STANDARD_COMMAND");
                String STR_Payload = incoming["data"].as<String>();
                cb->simple_CMD(STR_Payload);
            }else if(incoming["cmdType"] == "0x02"){
                Serial.println(" INFO_CM02 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Command 0x02 : FIRMWARE_UPDATE");
                String STR_Payload = incoming["data"].as<String>();
                cb->simple_OTA(STR_Payload);  
            }else if(incoming["cmdType"] == "0x03"){
                Serial.println(" INFO_CM02 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Command 0x03 : Module_UPDATE");
                String STR_Payload = incoming["data"].as<String>();
                setModuleRecievedCallback(STR_Payload);  
            }
        }
        else if(incoming["d"]["ct"] == 204){
              Device_Resp(str_payload);      
        }        
        else{
            Received_Cmd(str_payload);
        }
    }
}      // End IOTConnectClientHandler 



void IOTConnectClient::begin(const char hostname[], Client &client) { this->begin(hostname, 1883, client); }
void IOTConnectClient::begin(const char hostname[], int port, Client &client) {
    this->setHost(hostname, port);
    this->netClient = &client;
    lwmqtt_init(&this->client, this->writeBuf, this->bufSize, this->readBuf, this->bufSize);
    lwmqtt_set_timers(&this->client, &this->timer1, &this->timer2, lwmqtt_arduino_timer_set, lwmqtt_arduino_timer_get);
    lwmqtt_set_network(&this->client, &this->network, lwmqtt_arduino_network_read, lwmqtt_arduino_network_write);
    lwmqtt_set_callback(&this->client, (void *)&this->callback, IOTConnectClientHandler);
}


void IOTConnectClient::onMessageAdvanced(IOTConnectClientCallbackAdvanced cb) {
    this->callback.client = this;
    this->callback.simple_CMD = nullptr;
    this->callback.advanced = cb;
}


void IOTConnectClient::setHost(const char hostname[]) { this->setHost(hostname, 1883); }
void IOTConnectClient::setHost(const char hostname[], int port) {
    if (this->hostname != nullptr) {
        free((void *)this->hostname);
    }
    this->hostname = strdup(hostname);
    this->port = port;
}


void IOTConnectClient::setWill(const char topic[]) { this->setWill(topic, ""); }
void IOTConnectClient::setWill(const char topic[], const char payload[]) { this->setWill(topic, payload, false, 0); }
void IOTConnectClient::setWill(const char topic[], const char payload[], bool retained, int qos) {
    if (topic == nullptr || strlen(topic) == 0) {
        return;
    }
    this->clearWill();
    this->will = (lwmqtt_will_t *)malloc(sizeof(lwmqtt_will_t));
    memset(this->will, 0, sizeof(lwmqtt_will_t));
    this->will->topic = lwmqtt_string(strdup(topic));
    if (payload != nullptr && strlen(payload) > 0) {
        this->will->payload = lwmqtt_string(strdup(payload));
    }
    this->will->retained = retained;
    this->will->qos = (lwmqtt_qos_t)qos;
}


void IOTConnectClient::clearWill() {
    if (this->will == nullptr) {
        return;
    }
    if (this->will->payload.len > 0) {
        free(this->will->payload.data);
    }
    if (this->will->topic.len > 0) {
      free(this->will->topic.data);
    }
    free(this->will);
    this->will = nullptr;
}

  
void IOTConnectClient::setOptions(int keepAlive, bool cleanSession, int timeout) {
    this->keepAlive = (uint16_t)keepAlive;
    this->cleanSession = cleanSession;
    this->timeout = (uint32_t)timeout;
}


bool IOTConnectClient::connect(const char clientId[]) { return this->connect(clientId, nullptr, nullptr); }
bool IOTConnectClient::connect(const char clientId[], const char username[]) { return this->connect(clientId, username, nullptr); }
bool IOTConnectClient::connect(const char clientId[], const char username[], const char password[]) {
    bool skip = false;
    if (!skip && this->connected()) {
        this->close();
    }
    this->network.client = this->netClient;
    if(!skip) {
        int ret = this->netClient->connect(this->hostname, (uint16_t)this->port);
        if (ret <= 0) {
            return false;
        }
    }
    lwmqtt_options_t options = lwmqtt_default_options;
    options.keep_alive = this->keepAlive;
    options.clean_session = this->cleanSession;
    options.client_id = lwmqtt_string(clientId);
    if (username != nullptr) {
        options.username = lwmqtt_string(username);
        if (password != nullptr) {
            options.password = lwmqtt_string(password);
        }
    }
    this->_lastError = lwmqtt_connect(&this->client, options, this->will, &this->_returnCode, this->timeout);
    if (this->_lastError != LWMQTT_SUCCESS) {
        this->close();
        return false;
    }
    this->_connected = true;
    
    return true;
}


bool IOTConnectClient::publish(const String &topic) { return this->publish(topic.c_str(), ""); }
bool IOTConnectClient::publish(const char topic[]) { return this->publish(topic, ""); }
bool IOTConnectClient::publish(const String &topic, const String &payload) { return this->publish(topic.c_str(), payload.c_str()); }
bool IOTConnectClient::publish(const String &topic, const String &payload, bool retained, int qos) {
    return this->publish(topic.c_str(), payload.c_str(), retained, qos);    }
bool IOTConnectClient::publish(const char topic[], const String &payload) { return this->publish(topic, payload.c_str()); }
bool IOTConnectClient::publish(const char topic[], const String &payload, bool retained, int qos) {
    return this->publish(topic, payload.c_str(), retained, qos);    }
bool IOTConnectClient::publish(const char topic[], const char payload[]) {
    return this->publish(topic, (char *)payload, (int)strlen(payload));   }
bool IOTConnectClient::publish(const char topic[], const char payload[], bool retained, int qos) {
    return this->publish(topic, (char *)payload, (int)strlen(payload), retained, qos);    }
bool IOTConnectClient::publish(const char topic[], const char payload[], int length) {
    return this->publish(topic, payload, length, false, 0);   }
bool IOTConnectClient::publish(const char topic[], const char payload[], int length, bool retained, int qos) {
    if (!this->connected()) {
        return false;
    }
    lwmqtt_message_t message = lwmqtt_default_message;
    message.payload = (uint8_t *)payload;
    message.payload_len = (size_t)length;
    message.retained = retained;
    message.qos = lwmqtt_qos_t(qos);
    this->_lastError = lwmqtt_publish(&this->client, lwmqtt_string(topic), message, this->timeout);
    if (this->_lastError != LWMQTT_SUCCESS) {
        this->close();
        return false;
    }

    return true;
}


bool IOTConnectClient::subscribe(const String &topic) { return this->subscribe(topic.c_str()); }
bool IOTConnectClient::subscribe(const String &topic, int qos) { return this->subscribe(topic.c_str(), qos); }
bool IOTConnectClient::subscribe(const char topic[]) { return this->subscribe(topic, 0); }
bool IOTConnectClient::subscribe(const char topic[], int qos) {
    if (!this->connected()) {
        return false;
    }
    this->_lastError = lwmqtt_subscribe_one(&this->client, lwmqtt_string(topic), (lwmqtt_qos_t)qos, this->timeout);
    if (this->_lastError != LWMQTT_SUCCESS) {
        this->close();
        return false;
    }
    return true;
}


bool IOTConnectClient::unsubscribe(const String &topic) { return this->unsubscribe(topic.c_str()); }
bool IOTConnectClient::unsubscribe(const char topic[]) {
    if (!this->connected()) {
        return false;
    }
    this->_lastError = lwmqtt_unsubscribe_one(&this->client, lwmqtt_string(topic), this->timeout);
    if (this->_lastError != LWMQTT_SUCCESS) {
        this->close();
        return false;
    }
    return true;
}

bool IOTConnectClient::MQTT_loop() {
    if (!this->connected()) {
        return false;
    }
    auto available = (size_t)this->netClient->available();
    if (available > 0) {
        this->_lastError = lwmqtt_yield(&this->client, available, this->timeout);
        if (this->_lastError != LWMQTT_SUCCESS) {
            this->close();
            return false;
        }
    }
    this->_lastError = lwmqtt_keep_alive(&this->client, this->timeout);
    if (this->_lastError != LWMQTT_SUCCESS) {
        this->close();
        return false;
    }
    return true;
}


bool IOTConnectClient::connected() {
    return this->netClient != nullptr && this->netClient->connected() == 1 && this->_connected;
}

  
bool IOTConnectClient::disconnect() {
    if (!this->connected()) {
        return false;
    }
    this->_lastError = lwmqtt_disconnect(&this->client, this->timeout);
    this->close();
    return this->_lastError == LWMQTT_SUCCESS;
}


int IOTConnectClient::Init(String SID, String DID, String SdkOptions)
{
    if(SID || DID){
        if(SID == 0){
            Serial.println(" ERR_IN04 [ "+SID+" - "+DID+" ] : SID can not be blank");
            return true;
        }
        if(DID == 0){
            Serial.println(" ERR_IN05 [ "+SID+" - "+DID+" ] : UniqueId can not be blank");
            return true;
        }
    }

    String SyncResponseData;
    G_IoT_SID = SID;
    G_IoT_DID = DID;

    if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
        delay(100);
        DynamicJsonDocument in_SdkOptions(1024);
        deserializeJson(in_SdkOptions, SdkOptions); 
        IsDebug = in_SdkOptions["IsDebug"].as<bool>();
        if(IsDebug){
            Serial.println(" INFO_IN04 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Initializing.... ");
            Serial.println(" INFO_IN05 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Connecting.... ");
        }
    
        HTTPClient HTTP_D;    
        String discovery_url = in_SdkOptions["discoveryUrl"].as<String>();
        String url = "https://"+String(discovery_url)+"/api/sdk/sid/"+G_IoT_SID+"/uid/"+G_IoT_DID ;
        HTTP_D.begin(url); //Specify the URL and certificate
        int HTTPCode = HTTP_D.GET(); 
        delay(50);    
        String payload = HTTP_D.getString();     
        if( payload.length() >0) {
            StaticJsonDocument<512> root;
            deserializeJson(root, payload);
            G_base_url = root["data"]["bu"].as<String>();  
            Serial.println(" INFO_IN07 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : BaseUrl received to sync the device information");
        }else{
            Serial.println(" ERR_IN09 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Unable to get baseUrl");
        }
        HTTP_D.end();         
        SyncResponseData = Make_Sync_Call(G_base_url);
      }else{
        Serial.println(" ERR_IN08 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Network connection error or invalid url");
        return true;
      }
      Save_SYNC_Respo(SyncResponseData);
      return false;
}


void IOTConnectClient::Save_SYNC_Respo(String SyncResponseData){
  
    DynamicJsonDocument Sync_Res_Json(4097);
    Serial.println(SyncResponseData);
    DeserializationError error = deserializeJson(Sync_Res_Json, SyncResponseData); 
    if (error){
      Serial.println(" INFO_ [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Sync Response not JSON Data");
      return ;
    }
    else{
        G_Sync_Resp_info.ec = Sync_Res_Json["data"]["ec"].as<int>();
        G_Sync_Resp_info.device_tg = Sync_Res_Json["data"]["meta"]["dtg"].as<String>();
        G_Sync_Resp_info.tg = Sync_Res_Json["data"]["meta"]["tg"].as<String>();
        G_Sync_Resp_info.at = Sync_Res_Json["data"]["meta"]["at"].as<int>();
        G_Sync_Resp_info.DF = Sync_Res_Json["data"]["meta"]["df"].as<int>(); 
        G_Sync_Resp_info.Broker.Name = Sync_Res_Json["data"]["p"]["n"].as<String>();       
        G_Sync_Resp_info.Broker.Host = Sync_Res_Json["data"]["p"]["h"].as<String>();    
        G_Sync_Resp_info.Broker.ClientID = Sync_Res_Json["data"]["p"]["id"].as<String>();   
        G_Sync_Resp_info.Broker.UserName = Sync_Res_Json["data"]["p"]["un"].as<String>();    
        G_Sync_Resp_info.Broker.Password = Sync_Res_Json["data"]["p"]["pwd"].as<String>(); 
        G_Sync_Resp_info.Broker.PubTopic = Sync_Res_Json["data"]["p"]["pub"].as<String>();   
        G_Sync_Resp_info.Broker.SubTopic = Sync_Res_Json["data"]["p"]["sub"].as<String>();
        G_Sync_Resp_info.has.d = Sync_Res_Json["data"]["has"]["d"].as<int>();
        G_Sync_Resp_info.has.r = Sync_Res_Json["data"]["has"]["r"].as<int>();
        G_Sync_Resp_info.has.att = Sync_Res_Json["data"]["has"]["attr"].as<int>();
        G_Sync_Resp_info.has.sett = Sync_Res_Json["data"]["has"]["set"].as<int>();
        G_Sync_Resp_info.has.ota = Sync_Res_Json["data"]["has"]["ota"].as<int>();
      if(IsDebug){
          if(G_Sync_Resp_info.ec == 0)
              Serial.println(" INFO_IN08 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Response Code : 0 'OK'");
          else if(G_Sync_Resp_info.ec == 1)
              Serial.println(" INFO_IN09 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Response Code : 1 'DEVICE_NOT_REGISTERED'");
          else if(G_Sync_Resp_info.ec == 2)
              Serial.println(" INFO_IN10 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Response Code : 2 'AUTO_REGISTER'");
          else if(G_Sync_Resp_info.ec == 3)
              Serial.println(" INFO_IN11 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Response Code : 3 'DEVICE_NOT_FOUND'");
          else if(G_Sync_Resp_info.ec == 4)
              Serial.println(" INFO_IN12 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Response Code : 4 'DEVICE_INACTIVE'");
          else if(G_Sync_Resp_info.ec == 5)
              Serial.println(" INFO_IN13 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Response Code : 5 'OBJECT_MOVED'");
          else if(G_Sync_Resp_info.ec == 6)
              Serial.println(" INFO_IN14 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Response Code : 6 'CPID_NOT_FOUND'");
          else{
              Serial.println(" ERR_IN10 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Device information not found");
              Serial.println(" INFO_IN15 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Response Code : 'NO_RESPONSE_CODE_MATCHED'");
          }
      }
      delay(100);
    }
    Sync_Res_Json.clear();
}  //End save_data_SRensponse 



void IOTConnectClient::Connect(IOTConnectClientCallbackSimple setDeviceCommandCallback, IOTConnectClientCallbackSimple setOTAReceivedCallback, IOTConnectClientCallbackSimple setTwinChangedCallback){
      MQTT_Connection();
    
      // CallBack function
      this->callback.client = this;
      this->callback.simple_CMD = setDeviceCommandCallback;
      this->callback.simple_OTA = setOTAReceivedCallback;
      this->callback.simple_TWN = setTwinChangedCallback;
      this->callback.advanced = nullptr;
      if(connected()){
         if(IsDebug) 
            Serial.println(" INFO_CM09 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Command 0x16 : SDK_CONNECTION_STATUS");            
        }
        String msg="{\"cpid\":\""+G_IoT_SID+"\",\"guid\":\"\",\"uniqueId\":\""+G_IoT_DID+"\",\"command\":true,\"ack\":false,\"ackId\":\"\",\"cmdType\":\"0x16\"}";;
        setConnectionStatusChangedCallback(msg);
       if(G_Sync_Resp_info.has.d){
           String info = Calling_Type(204);
           Send_to_Azure(info);
           }
       delay(5000);
}


String IOTConnectClient::Make_Sync_Call(String G_base_url){

    HTTPClient HTTP_A;   
    HTTP_A.begin(G_base_url); 
    int HTTPCode = HTTP_A.GET();
    String Payload = "";  
    if (HTTPCode>0) {
        Payload = HTTP_A.getString();
    }else {
        Serial.print("Error code: ");
        Serial.println(HTTPCode);
    }
    HTTP_A.end();      
    return Payload;
}


void IOTConnectClient::MQTT_Connection(){  
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
        }
    begin(string2char(G_Sync_Resp_info.Broker.Host), 8883, Net_MQTT);
    delay(50);
    while (!connect(string2char(G_Sync_Resp_info.Broker.ClientID), string2char(G_Sync_Resp_info.Broker.UserName), string2char(G_Sync_Resp_info.Broker.Password) )) {
        Serial.print(".");
        delay(800);
    }
    subscribe(string2char(G_Sync_Resp_info.Broker.SubTopic),  1);
    subscribe(string2char(twinPropertySubTopic), 0);
    subscribe(string2char(twinResponseSubTopic), 0);
} //End MQTTConnection


void IOTConnectClient::GetAllTwins(void){
  delay(100);
  if (!connected())
      MQTT_Connection();
  if(publish(string2char(twinResponsePubTopic), " ", 0))
      Serial.println(" INFO_TP02 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Request sent successfully to get the all twin properties");
}


void IOTConnectClient::HTTP_Connection(String incoming){
    String PayLoad ="";
    String urlX= "https://"+G_Sync_Resp_info.Broker.Host+"/devices/"+G_Sync_Resp_info.Broker.ClientID+"/messages/events?api-version="+httpAPIVersion;
    if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
            HTTPClient http;    //Declare object of class HTTPClient
            hr:
            http.begin(urlX);      //Specify request destination
            http.addHeader("Accept", "application/json");  //Specify content-type header
            http.addHeader("Content-Type", "application/json");
            http.addHeader("authorization", G_Sync_Resp_info.Broker.Password);
            int httpCode = http.POST(incoming);   //Send the request
            PayLoad = http.getString();                  //Get the response payload 
            if(httpCode >0)
              Serial.println("\t\tHTTPS published\n\n");
            else 
                goto hr;
        } 
}   //END of HTTP_Connection

// this function will get data for IoTConnect
void IOTConnectClient::SendData(String Attribute_json_Data){
    if(Flag_99){
        Serial.println(" INFO_DC01 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Device already disconnected");
        return ;
    }
    else{
        int x=0,y=0;
        time_t now = time(nullptr);
        String time_now = (ctime(&now));
        int time_diff = now - G_Last_DF;
        Serial.println(time_diff);
        if( G_Sync_Resp_info.DF < time_diff){
            
            if(!G_Sync_Resp_info.ec){
              
                String in_time[10],in_ID[10];
                JsonObject in_Attr[10];
                delay(50); 
                DynamicJsonDocument Attribute_json(2048);
                deserializeJson(Attribute_json, Attribute_json_Data);
                JsonArray root = Attribute_json.as<JsonArray>();    
                int Asize = root.size();
                for(int x = 0; x < Asize ;x++){    
                    JsonObject row = root[x].as<JsonObject>();;
                    in_ID[x] = row["uniqueId"].as<String>();
                    in_time[x] = row["time"].as<String>();
                    in_Attr[x] = row["data"].as<JsonObject>();
 
                    for (JsonPair keyValue : in_Attr[x]) {
                        String KEY = keyValue.key().c_str();
                        String blk =keyValue.value().as<char*>();
                        if(in_Attr[x][KEY].is<const char*>()){
                            if((blk == "") ||(blk == " ")){
                                in_Attr[x].remove(keyValue.key());
                            }
                        }
                    }
                }
                              
                DynamicJsonDocument To_HUB_json(2048);
                  To_HUB_json["sid"] = G_IoT_SID;
                  To_HUB_json["dtg"] = G_Sync_Resp_info.device_tg;
                  To_HUB_json["dt"] = in_time[0];
                  To_HUB_json["mt"] = 0;
                  JsonArray device = To_HUB_json.createNestedArray("d");
                  for(x = 0; x < Asize ;x++){
                      JsonObject Device = device.createNestedObject();
                      Device["id"] = in_ID[x];
                      for (y=0; y<10; y++){
                          if(in_ID[x] == G_Sync_Resp_info.dvc_ID[y]){
                            Device["tg"] = G_Sync_Resp_info.dvc_TG[y];
                            break;
                          }
                          else{
                              Device["tg"] = G_Sync_Resp_info.tg;
                              continue;
                              }                        
                      }
                      Device["dt"] = in_time[0];
                      Device["d"] = in_Attr[x];
                  }
                  String To_HUB_json_Data;
                  serializeJson(To_HUB_json, To_HUB_json_Data);
                  Serial.println(To_HUB_json_Data);
                  int error = Send_to_Azure(To_HUB_json_Data);
                  if(error){
                      Serial.println(" ERR_SD10 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Publish data failed : MQTT connection not found");
                  }
                  else
                      Serial.println(" INFO_SD01 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Publish Data");    
                  delay(100);
                  To_HUB_json.clear();
                  }
        G_Last_DF = now;
        }
    }
} //End SendDataINput

//This will UpdateTwin property to IoTConnect
void IOTConnectClient::UpdateTwin(String key,String value){
   String Twin_Json_Data;
   DynamicJsonDocument root(256);  
   root[key] = value;
   serializeJson(root, Twin_Json_Data);
   Serial.println(Twin_Json_Data);
   root.clear(); 
   if (!connected())
      MQTT_Connection();
   if(publish(string2char(twinPropertyPubTopic), Twin_Json_Data, 0, 1))
      Serial.println(" INFO_TP01 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Twin property updated successfully");
}

char* IOTConnectClient::string2char(String command){
    if(command.length()!=0){
        char *p = const_cast<char*>(command.c_str());
        return p;
    }
}


// Send_to_Azure send data to HUB by .... Protocol
int IOTConnectClient::Send_to_Azure( String pubData){
    if(G_Sync_Resp_info.Broker.Name == "http" ||G_Sync_Resp_info.Broker.Name == "https"){
        HTTP_Connection(pubData);
    }
    else if(G_Sync_Resp_info.Broker.Name == "mqtt" ||G_Sync_Resp_info.Broker.Name == "mqtts"){
        if (!connected())
            MQTT_Connection();
        if(publish(string2char(G_Sync_Resp_info.Broker.PubTopic), pubData , 0, 1)){
            delay(10);
            return 0;            
        }else{
            return 1;
        }
    }
    else
        Serial.println(" ERR_SD11 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Unknown broker protocol");
}

bool IOTConnectClient::Dispose(){
    if (disconnect()){
    Flag_99 = true; 
        Serial.println("\n\t:: Device: [ "+G_IoT_SID+" - "+G_IoT_DID+" ] is barred - sendData() not permitted. ::");
    }
}

// this will send the ACK of receiving OTA
void IOTConnectClient::sendAckOta(int success, String Ack_FW_Data){
  
    int Status =0; String cmd_ackID = "";
    StaticJsonDocument<228> Ack_FW;
    deserializeJson(Ack_FW, Ack_FW_Data);
    cmd_ackID = Ack_FW["ackId"].as<String>();
    StaticJsonDocument<512> Ack_Json;
    Ack_Json["uniqueId"] = G_IoT_DID;
    Ack_Json["sid"] = G_IoT_SID;
    Ack_Json["mt"] = 11;
    Ack_Json["dt"] = GetTime();
    JsonObject inData = Ack_Json.createNestedObject("d");
        inData["ackId"] = cmd_ackID;
        inData["msg"] = "";
        inData["childId"] = "";
        (success)?(Status =7):(Status =4);
        inData["st"] = Status;
 
    String Ack_Json_Data;
    serializeJson(Ack_Json, Ack_Json_Data);
    int error = Send_to_Azure(Ack_Json_Data);
    if(error){
      Serial.println(" ERR_CM10 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : OTAReceived acknowledgement Failed");
      }
    else
        Serial.println(" INFO_CM10 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : OTAReceived acknowledgement success");
    Ack_Json.clear();    Ack_FW.clear();  
}

// this will send the ACK of receiving Commands
void IOTConnectClient::sendAckCmd(int success, String Ack_FW_Data){
  
    int Status =0; String cmd_ackID = "";
    StaticJsonDocument<228> Ack_FW;
    deserializeJson(Ack_FW, Ack_FW_Data);
    cmd_ackID = Ack_FW["ackId"].as<String>();
    StaticJsonDocument<512> Ack_Json;
    Ack_Json["uniqueId"] = G_IoT_DID;
    Ack_Json["sid"] = G_IoT_SID;
    Ack_Json["mt"] = 5;
    Ack_Json["dt"] = GetTime();
    JsonObject inData = Ack_Json.createNestedObject("d");
        inData["ackId"] = cmd_ackID;
        inData["msg"] = "";
        inData["childId"] = "";
        (success)?(Status =6):(Status =4);
        inData["st"] = Status;
 
    String Ack_Json_Data;
    serializeJson(Ack_Json, Ack_Json_Data);
    int error = Send_to_Azure(Ack_Json_Data);
    if(error){
      Serial.println(" ERR_CM10 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Command acknowledgement Failed");
   }
    else
      Serial.println(" INFO_CM10 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Command acknowledgement success");
    Ack_Json.clear();    Ack_FW.clear();  
}



void Device_Resp(String payload){
    
    DynamicJsonDocument device_Json(2048);
    DeserializationError error = deserializeJson(device_Json, payload); 
    if (!error){
       JsonArray root = device_Json["d"]["d"].as<JsonArray>();    
       int Asize = root.size();
       for(int x = 0; x < Asize ;x++){    
          JsonObject row = root[x].as<JsonObject>();
          G_Sync_Resp_info.dvc_TG[x] = row["tg"].as<String>();
          G_Sync_Resp_info.dvc_ID[x] = row["id"].as<String>();
       } 
    } 
}

/*! Object  - will get the SDK command from C2D
 *  \param  - Payload of command
 *  \return - None
 */
 
void Received_Cmd(String payload){

    String PayLoad="",Cmd_value="";  
    StaticJsonDocument<512> incmd;   
    DeserializationError error = deserializeJson(incmd, payload);
    if (!error){
        if(incmd["cmdType"].as<String>() == "0x12"){
            Serial.println(" INFO_CM05 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Command : 0x12 : PASSWORD_UPDATE");
            SDKclient.disconnect();                delay(10000);
            PayLoad = SDKclient.Make_Sync_Call(G_base_url);
            SDKclient.Save_SYNC_Respo(PayLoad);
            SDKclient.MQTT_Connection();
        }
        else if(incmd["cmdType"].as<String>() == "0x10")
            Serial.println(" INFO_CM03 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Command 0x10 : ATTRIBUTE_UPDATE");     
        else if(incmd["cmdType"].as<String>() == "0x11")
             Serial.println(" INFO_CM04 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Command 0x11 : SETTING_UPDATE");
        else if(incmd["cmdType"].as<String>() == "0x13")
             Serial.println(" INFO_CM06 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Command 0x13 : DEVICE_UPDATE");
        else if(incmd["cmdType"].as<String>() == "0x15")
             Serial.println(" INFO_CM07 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Command 0x15 : RULE_UPDATE");
        else if(incmd["cmdType"].as<String>() == "0x99"){
             SDKclient.disconnect(); SDKclient.Flag_99 = true;   
             Serial.println(" INFO_CM08 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Command 0x99 : STOP_SDK_CONNECTION");
            }
        else if(incmd["cmdType"].as<String>() == "0x17"){
             PayLoad = SDKclient.Make_Sync_Call(G_base_url);
             DynamicJsonDocument Sync_Res_Json(4097);
             DeserializationError error = deserializeJson(Sync_Res_Json, PayLoad);
             G_Sync_Resp_info.DF = Sync_Res_Json["data"]["meta"]["df"].as<int>();    
             Serial.println(" INFO_CM08 [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Command 0x17 : DF_UPDATE");
            }
        else
             Serial.println(" INFO_CM [ "+G_IoT_SID+" - "+G_IoT_DID+" ] : Command      :  'NO_COMMAND_CODE_MATCHED'");
    }
}


/*! Object  - will gave JSON for SYNC and device call with "MT" type
 *  \param  - MT { "allStatus" : 200, /"attribute" : 201, /"setting" : 202,
 *	 		       "rule" : 203, /"childDevice" : 204, /"all" : 210 }
 *  \return - data for publish
 */
String IOTConnectClient::Calling_Type(int MT){

   StaticJsonDocument<512> Ack_Json;
   Ack_Json["t"] = GetTime();
   Ack_Json["sid"] = G_IoT_SID;
   Ack_Json["mt"] = MT;
   Ack_Json["v"] = 1;

   String Ack_Json_Data;
   serializeJson(Ack_Json, Ack_Json_Data);
   return Ack_Json_Data;
}
