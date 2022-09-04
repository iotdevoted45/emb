
#ifndef IoT_CLIENT_H
#define IoT_CLIENT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Client.h>
#include <Stream.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

extern "C" {
#include "lwmqtt.h"
};
typedef struct {
  uint32_t end;
} lwmqtt_arduino_timer_t;

typedef struct {
  Client *client;
} lwmqtt_arduino_network_t;

inline void lwmqtt_arduino_timer_set(void *ref, uint32_t timeout) {
    auto t = (lwmqtt_arduino_timer_t *)ref;
    t->end = (uint32_t)(millis() + timeout);
}

inline int32_t lwmqtt_arduino_timer_get(void *ref) {
    auto t = (lwmqtt_arduino_timer_t *)ref;
    return (int32_t)t->end - (int32_t)millis();
}

inline lwmqtt_err_t lwmqtt_arduino_network_read(void *ref, uint8_t *buffer, size_t len, size_t *read, uint32_t timeout) {
    auto n = (lwmqtt_arduino_network_t *)ref;
    n->client->setTimeout(timeout);
    *read = n->client->readBytes(buffer, len);
    if (*read <= 0) {
        return LWMQTT_NETWORK_FAILED_READ;
    }
    return LWMQTT_SUCCESS;
}

inline lwmqtt_err_t lwmqtt_arduino_network_write(void *ref, uint8_t *buffer, size_t len, size_t *sent, uint32_t /*timeout*/) {
    auto n = (lwmqtt_arduino_network_t *)ref;
    *sent = n->client->write(buffer, len);
    if (*sent <= 0) {
        return LWMQTT_NETWORK_FAILED_WRITE;
    }
    return LWMQTT_SUCCESS;
}

class IOTConnectClient;

typedef void (*IOTConnectClientCallbackSimple)(String &payload);
typedef void (*IOTConnectClientCallbackAdvanced)(IOTConnectClient *client, char topic[], char bytes[], int length);

typedef struct {
    IOTConnectClient *client = nullptr;
    IOTConnectClientCallbackSimple simple_CMD = nullptr;
    IOTConnectClientCallbackSimple simple_OTA = nullptr;
    IOTConnectClientCallbackSimple simple_TWN = nullptr;
    IOTConnectClientCallbackAdvanced advanced = nullptr;
} 
IOTConnectClientCallback;

void Device_Resp(String payload);
void Received_Cmd(String payload);

class IOTConnectClient {  
private:

        char* const httpAPIVersion = "2016-02-03";
        
        String twinPropertyPubTopic ="$iothub/twin/PATCH/properties/reported/?$rid=1";
        String twinPropertySubTopic ="$iothub/twin/PATCH/properties/desired/#";
        String twinResponsePubTopic ="$iothub/twin/GET/?$rid=0";
        String twinResponseSubTopic ="$iothub/twin/res/#";
        
        size_t bufSize = 0;
        uint8_t *readBuf = nullptr;
        uint8_t *writeBuf = nullptr;
      
        uint16_t keepAlive = 10;
        bool cleanSession = true;
        uint32_t timeout = 10000;

        Client *netClient = nullptr;
        const char *hostname = nullptr;
        int port = 0;
        lwmqtt_will_t *will = nullptr;
        IOTConnectClientCallback callback;
      
        lwmqtt_arduino_network_t network = {nullptr};
        lwmqtt_arduino_timer_t timer1 = {0};
        lwmqtt_arduino_timer_t timer2 = {0};
        lwmqtt_client_t client;
      
        bool _connected = false;
        lwmqtt_return_code_t _returnCode = (lwmqtt_return_code_t)0;
        lwmqtt_err_t _lastError = (lwmqtt_err_t)0;
          
        void close() {
          this->_connected = false;
          this->netClient->stop();
          }
        bool connect(const char clientId[]);
        bool connect(const char clientId[], const char username[]);
        bool connect(const char clientId[], const char username[], const char password[]);        
        lwmqtt_err_t lastError() { return this->_lastError; }
        lwmqtt_return_code_t returnCode() { return this->_returnCode; }
        void begin(const char hostname[], Client &client);
        void begin(const char hostname[], int port, Client &client);
        void onMessageAdvanced(IOTConnectClientCallbackAdvanced cb) ;
        void setHost(const char hostname[]);
        void setHost(const char hostname[], int port);
        void setWill(const char topic[]);
        void setWill(const char topic[], const char payload[]);
        void setWill(const char topic[], const char payload[], bool retained, int qos); 
        void clearWill();
        void setOptions(int keepAlive, bool cleanSession, int timeout);
        bool subscribe(const String &topic);
        bool subscribe(const String &topic, int qos);
        bool subscribe(const char topic[]);
        bool subscribe(const char topic[], int qos);
        bool unsubscribe(const String &topic);
        bool unsubscribe(const char topic[]);
        boolean connectwifi(char* ssid, char* password);
        bool publish(const String &topic);
        bool publish(const char topic[]);
        bool publish(const String &topic, const String &payload);
        bool publish(const String &topic, const String &payload, bool retained, int qos);
        bool publish(const char topic[], const String &payload);
        bool publish(const char topic[], const String &payload, bool retained, int qos);
        bool publish(const char topic[], const char payload[]);
        bool publish(const char topic[], const char payload[], bool retained, int qos);
        bool publish(const char topic[], const char payload[], int length);
        bool publish(const char topic[], const char payload[], int length, bool retained, int qos);
        bool connected();
         
 public: 
    explicit IOTConnectClient(int bufSize = 4098) {
        memset(&this->client, 0, sizeof(lwmqtt_client_t));
        this->bufSize = (size_t)bufSize;
        this->readBuf = (uint8_t *)malloc((size_t)bufSize + 1);
        this->writeBuf = (uint8_t *)malloc((size_t)bufSize);
    }

    ~IOTConnectClient() {
        this->clearWill();
        if (this->hostname != nullptr) {
            free((void *)this->hostname);
        }
        free(this->readBuf);
        free(this->writeBuf);
    }
    bool Flag_99 = false; 
    bool IsDebug = false;
    bool disconnect();
    bool MQTT_loop();
    bool Dispose();        
    void MQTT_Connection(); 
    boolean Wifi_connect();
    String Calling_Type(int MT);
    char* string2char(String command);
    void Save_SYNC_Respo(String SyncResponseData);
    void HTTP_Connection(String incoming);
    String Make_Sync_Call(String G_base_url);
    int Send_to_Azure( String pubData);
    void SendData(String Attribute_json_Data);  
    void UpdateTwin(String key,String value);
    void sendAckOta(int success, String Ack_FW_Data);
    void sendAckCmd(int success, String Ack_FW_Data);
    void GetAllTwins(void);
    int Init(String SID, String DID, String SdkOptions); 
    void Connect(IOTConnectClientCallbackSimple setDeviceCommandCallback, IOTConnectClientCallbackSimple setOTAReceivedCallback, IOTConnectClientCallbackSimple setTwinChangedCallback);

};

#endif      //IoT_CLIENT_H
