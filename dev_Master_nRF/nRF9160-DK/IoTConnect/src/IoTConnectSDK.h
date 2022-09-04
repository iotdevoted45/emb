#ifndef IOTCONNECTSDK_H
#define IOTCONNECTSDK_H


#include <zephyr.h>
#include <cJSON.h>
#include <time.h>
#define IOTCONNECT_SERVER_MQTT_PORT         8883
#define IOTCONNECT_SERVER_HTTP_PORT         443
#define MAXLINE 4096
#define TLS_SEC_TAG 42

void MQTT_Init(void);
int tls_setup(int fd);
void GetAllTwins(void);
int subscribe(void);
void broker_init(void);
void Received_cmd(char *in_cmd);
void UpdateTwin(char *key, char *value);
int provision_certificates(void);
int GetTimeDiff(char newT[25], char oldT[25]);
void Save_Sync_Responce(char *sync_data);
void SendData(char *Attribute_json_Data);
int fds_init(struct mqtt_client *c);
void TwinUpdateCallback(char *payload);
void DeviceCallback(char *payload);
void client_init(struct mqtt_client *client);
char *get_base_url(char*Host, char *cpid, char *env);
char *Sync_call(char *cpid, char *uniqueid, char *base_url);
void SendAck(char *Ack_Data, int messageType);
typedef void (*IOTConnectCallback)(char *PayLoad);
void data_print(u8_t *prefix, u8_t *data, char *topic, size_t len);
void mqtt_evt_handler(struct mqtt_client *const c, const struct mqtt_evt *evt);
int data_publish(struct mqtt_client *c, char *topic, enum mqtt_qos qos, u8_t *data, size_t len);
int IoTConnect_init(char *CpID, char *UniqueID, IOTConnectCallback CallBack, IOTConnectCallback TwinCallBack, char *sdkoption, char *Env);
void IoTConnect_connect();
int IoTConnect_abort();
#endif /* IOTCONNECTSDK_H */
