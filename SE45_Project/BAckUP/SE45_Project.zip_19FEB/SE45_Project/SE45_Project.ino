
#include "project_config.h"
#include "All_stepper.h"
#include "HTML.h"
#include <WebServer.h>
#include <HTTPClient.h>

//////////////////////////////////////////////
//   EEPROM Address for save 
int EE_SSID_ADD = 210;
int EE_PASS_ADD = 250;
int EE_LAST_ADD = 35; 
int EE_ITEM_ADD = 50;
WebServer server(80);

////////////////////////////////////////////
//    User define function 
void setup_wifi() ;
void Lift_ONE_DOWN(int Rq_delay);
void Lift_AT_A(int Rq_delay);
void Check_loc();
void liftSetup();
void CLOSE_D1();
void CLOSE_D2();
void Check_last_order();
void MOTOR_PUSH(bool Dir, int Rot);
void MOTOR_IN_D(bool Dir, int Rot);
void MOTOR_Ot_D(bool Dir, int Rot);
void newItem();
#define Isdebug true

//////////////////////////////////////////////
//    Flag for checking status 
bool RCV_ORDER_FLAG = false;
bool RCV_REFIL_FLAG = false;
bool RCV_NEW_ORDER  = false;
bool LIFT_UP = false;
bool LIFT_DOWN = false;

//////////////////////////////////////////////
//    GPIO OF ESP32 
unsigned int reBoot_PIN = 34;

#define EEPROM_SIZE 400   
unsigned int liftMAX= 15;
unsigned int liftMIN= 5;
unsigned int IN_DOOR= 18;
unsigned int OT_DOOR= 19;
unsigned int Push_limit = 4;
unsigned int Location_B = 25;
unsigned int Location_C = 33;
unsigned int Location_D = 35;
unsigned int Location_E = 32;
unsigned int Relay_P= 27;

unsigned int G_OrderID = 0;         // will received from SUb and use for update the status
unsigned int G_Qty[4] = {0};        // will received from SUb and use for update the status
String G_Loc[4] = {""};             // will received from SUb and use for update the status
unsigned int LASTORDER = 0;
unsigned long lastMillis = 0;
unsigned long currentMicros = 0; 
unsigned long previousMicros = 0;


////////////////////////////////////////////
int CurrentITEM[25] ={0};


#include "WiFiManager.h"
WiFiClient espClient;
PubSubClient client(espClient);

////////////////////////////////////////////
void setup() {
    pinMode(reBoot_PIN, INPUT);    digitalWrite(reBoot_PIN, HIGH); 
    Serial.begin(115200);  
    Serial.println(F(" :: Initialize System :: "));
    EEPROM.begin(EEPROM_SIZE);
    if(!CheckWIFICreds()){
        Serial.println("No WIFI credentials stored in memory. Loading form...");
        while(loadWIFICredsForm());
    }
      delay(2000);
    pinMode(Relay_P, OUTPUT);    digitalWrite(Relay_P, LOW);    

    pinMode(liftMAX,INPUT_PULLUP);    digitalWrite(liftMAX,HIGH);  
    pinMode(liftMIN,INPUT_PULLUP);    digitalWrite(liftMIN,HIGH); 
    pinMode(IN_DOOR,INPUT_PULLUP);    digitalWrite(IN_DOOR,HIGH);  
    pinMode(OT_DOOR,INPUT_PULLUP);    digitalWrite(OT_DOOR,HIGH);
    pinMode(Location_B,INPUT_PULLUP);    digitalWrite(Location_B,HIGH);  
    pinMode(Location_C,INPUT_PULLUP);    digitalWrite(Location_C,HIGH); 
    pinMode(Location_D,INPUT_PULLUP);    digitalWrite(Location_D,HIGH);  
    pinMode(Location_E,INPUT_PULLUP);    digitalWrite(Location_E,HIGH); 
    pinMode(Push_limit,INPUT_PULLUP);    digitalWrite(Push_limit,HIGH); 

    delay(2000);
    Wire.begin();
    mcpU2.init();     mcpU3.init();
    mcpU4.init();     mcpU5.init();
    mcpU6.init();     mcpU7.init();

    mcpU2.portMode(MCP23017Port::A, 0);           //Port A as output
    mcpU2.portMode(MCP23017Port::B, 0);           //Port B as output
    mcpU3.portMode(MCP23017Port::A, 0);           //Port A as output
    mcpU3.portMode(MCP23017Port::B, 0);           //Port B as output
    mcpU4.portMode(MCP23017Port::A, 0);           //Port A as output
    mcpU4.portMode(MCP23017Port::B, 0);           //Port B as output
    mcpU5.portMode(MCP23017Port::A, 0);           //Port A as output
    mcpU5.portMode(MCP23017Port::B, 0);           //Port B as output
    mcpU6.portMode(MCP23017Port::A, 0);           //Port A as output
    mcpU6.portMode(MCP23017Port::B, 0);           //Port B as output
    mcpU7.portMode(MCP23017Port::A, 0);           //Port A as output
    mcpU7.portMode(MCP23017Port::B, 0);           //Port B as output

    client.setServer(Broker, M_port);
    client.setCallback(callback);
    if(client.connect("Suthar_Electronics"))
        Serial.println(" :: MQTT connected :: ");
    client.subscribe(SUB_TP1);
    client.subscribe(SUB_TP2);
    delay(2000);
    liftSetup();      // it will set lift on GND if it not
    delay(1500);
    CLOSE_PUSH();
    delay(2000);
    CLOSE_D1();       // if open it will close the inner door
    delay(1500);
    CLOSE_D2();       // if open it will close the outer door
    delay(1500);

  mcpU6.writeRegister(MCP23017Register::GPIO_A, 0);
  mcpU6.writeRegister(MCP23017Register::GPIO_B, 0); 
  mcpU7.writeRegister(MCP23017Register::GPIO_A, 0); 
  mcpU7.writeRegister(MCP23017Register::GPIO_B, 0);  
//     Serial.println(" :: While 1 :: ");
// while(1);
}

////////////////////////////////////////////
//    it will set lift on GND
void liftSetup(){
  Serial.println(" :: Checking and Setup LIFT on GND :: ");
  for (long zz=0;zz<97655 ;zz++){
      int Delay = (zz < 10 )? 2999 : 820;
      Lift_ONE_DOWN(Delay);
      Serial.println(zz);
      if(!digitalRead(liftMIN)){         // if bymistak lift go on max low will go bit down 
          Serial.println(" :: Inside if :: ");
          for (int zzz=0;zzz<5;zzz++)
              Lift_AT_A(2999);
              break;
        }
    }
 Serial.println(" :: Now Lift on GND location :: ");

}

////////////////////////////////////////////
//    if open it will close the inner door
void CLOSE_D1(){
  Serial.println(" :: Checking and Closing the Door_1 :: ");
  mcpU7.writeRegister(MCP23017Register::GPIO_A, Motor_Door_1);  //Reset port B
  for(int S=0; S < (200*25); S++){
        LINE_X_FRD(0x20,0x00);          // LINE_X_RVC(0x30,0x10); 
        if(!digitalRead(IN_DOOR)){
          for(int SS=0; SS < (200*20); SS++) 
              LINE_X_RVC(0x30,0x10);    //LINE_X_FRD(0x20,0x00);  
          break;       
        }
  }    delay(20);
}

////////////////////////////////////////////
//    if open it will close the outer door
void CLOSE_D2(){
  Serial.println(" :: Checking and Closing the Door_2 :: ");  
  mcpU7.writeRegister(MCP23017Register::GPIO_A, Motor_Door_2);  //Reset port B
  for(int S=0; S < (200*28); S++){
        LINE_X_FRD(0x80,0x00);          //        LINE_X_RVC(0xC0,0x40); 
        if(!digitalRead(OT_DOOR)){
         for(int SS=0; SS < (200*26); SS++) 
           LINE_X_RVC(0xC0,0x40);    //          LINE_X_FRD(0x80,0x00);  
          break;       
        } 
  }    delay(20); 
}

////////////////////////////////////////////
//    if open it will close the outer door
void CLOSE_PUSH(){
  Serial.println(" :: Checking and Closing the PUSH_PLAT :: ");  
    mcpU7.writeRegister(MCP23017Register::GPIO_A, Motor_PUSH_0);  //Reset port B
    for(int S=0; S < (200*95); S++){
      Serial.println(S);
        LINE_X_RVC(0x0C,0x04); 
        if(!digitalRead(Push_limit))
          break;
    }
    delay(20);
}


////////////////////////////////////////////
//    We start by connecting to a WiFi network
void setup_wifi() {
  delay(10);      
  Serial.print("\n\nWifi Connecting to : ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    //digitalWrite(RED, LOW);
    delay(100); 
      if(digitalRead(reBoot_PIN) == LOW){
        Serial.println("Wiping WiFi credentials from memory...");
        wipeEEPROM();
        Serial.println("\n\nDevice Fully Reset!\n\n");
        Serial.println("\n\n** Restart after 5 Sec ! **\n\n");
        delay(5500);
        ESP.restart();
    }

  }

  Serial.println("\n\tWiFi connected");
  Serial.print("device IP: ");
  Serial.println(WiFi.localIP());
  delay(5000);

}

////////////////////////////////////////////
//   here we received all cloud msg
StaticJsonDocument<1024> Payload;
void callback(char* topic, byte* message, unsigned int length) {
    Serial.print(" >> On Topic: ");   Serial.println(topic);  
    Serial.print(" >> Message arrived : ");
    String  payload = String((char*)message).substring(0,length);
    Serial.println(payload);
    DeserializationError err = deserializeJson(Payload, payload);  
    if (err){
        Serial.println(" >> [Err] DeserializeJson() failed: ");
        return ; 
    }
    if(Payload.containsKey("order_id")){
          const char* value = 0; 
          G_OrderID = Payload["order_id"];
          if (Payload["qty"].size() == Payload["location"].size()){
              Serial.println(" >> [INFO]  Size of QTY and Location are same");
              for(int aa=0; aa<Payload["qty"].size(); aa++){
                G_Qty[aa] = Payload["qty"][aa]; 
                value = Payload["location"][aa];      
                G_Loc[aa] = String(value);
                }
              Serial.println(" .. Data saved  .. ");    
              RCV_ORDER_FLAG = true;             
          }else{
              Serial.println(" >> [Err] Array isn't with same size ... System Return");
              return ;
          }
    }else if(Payload.containsKey("cmd")){
          RCV_REFIL_FLAG = true;
          newItem();

    }else {
       Serial.print(" >> [Err] Invalid Message received from MQTT");
    }
    Payload.clear();
    return ;
}

////////////////////////////////////////////
//    this function is for update refill Item
void newItem(){
  
  Read_update_ITEM();
  Serial.println(" :: All Motors will move back as new Refill data :: ");
  for(int ITEM = 0; ITEM < (Payload["data"]["A1"].as<int>() - CurrentITEM[0]); ITEM++){
      mcpU6.writeRegister(MCP23017Register::GPIO_B, MotorA1);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_A_RVC(0x02,0x00);
          delay(20);
      }   
      CurrentITEM[0]  = Payload["data"]["A1"].as<int>();
      
  for(int ITEM = 0; ITEM < (Payload["data"]["A2"].as<int>() - CurrentITEM[1]); ITEM++){
      mcpU6.writeRegister(MCP23017Register::GPIO_B, MotorA2);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_A_RVC(0x08,0x00);
          delay(20);
      }   
      CurrentITEM[1]  = Payload["data"]["A2"].as<int>();

  for(int ITEM = 0; ITEM < (Payload["data"]["A3"].as<int>() - CurrentITEM[2]); ITEM++){
      mcpU6.writeRegister(MCP23017Register::GPIO_B, MotorA3);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_A_RVC(0x20,0x00);
          delay(20);
      }   
      CurrentITEM[2]  = Payload["data"]["A3"].as<int>();

  for(int ITEM = 0; ITEM < (Payload["data"]["A4"].as<int>() - CurrentITEM[3]); ITEM++){
      mcpU6.writeRegister(MCP23017Register::GPIO_B, MotorA4);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_A_RVC(0x80,0x00);
          delay(20);
      }   
      CurrentITEM[3]  = Payload["data"]["A4"].as<int>();
  for(int ITEM = 0; ITEM < (Payload["data"]["A5"].as<int>() - CurrentITEM[4]); ITEM++){
      mcpU7.writeRegister(MCP23017Register::GPIO_A, MotorA5);  //Reset port A
      for(int S=0; S < (200*Rotation); S++)
          LINE_5_RVC(0x40,0x00);
          delay(20);
      }
      CurrentITEM[4]  = Payload["data"]["A5"].as<int>();

  for(int ITEM = 0; ITEM < (Payload["data"]["B1"].as<int>() - CurrentITEM[5]); ITEM++){
      mcpU6.writeRegister(MCP23017Register::GPIO_B, MotorB1);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_B_RVC(0x40,0x00);
          delay(20);
      }      
      CurrentITEM[5]  = Payload["data"]["B1"].as<int>();

  for(int ITEM = 0; ITEM < (Payload["data"]["B2"].as<int>() - CurrentITEM[6]); ITEM++){
      mcpU6.writeRegister(MCP23017Register::GPIO_B, MotorB2);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_B_RVC(0x10,0x00);
          delay(20);
      }  
      CurrentITEM[6]  = Payload["data"]["B2"].as<int>();

  for(int ITEM = 0; ITEM < (Payload["data"]["B3"].as<int>() - CurrentITEM[7]); ITEM++){
      mcpU6.writeRegister(MCP23017Register::GPIO_B, MotorB3);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_B_RVC(0x04,0x00);
          delay(20);
      }  
      CurrentITEM[7]  = Payload["data"]["B3"].as<int>();

  for(int ITEM = 0; ITEM < (Payload["data"]["B4"].as<int>() - CurrentITEM[8]); ITEM++){
      mcpU6.writeRegister(MCP23017Register::GPIO_B, MotorB4);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_B_RVC(0x01,0x00);
          delay(20);
      }  
      CurrentITEM[8]  = Payload["data"]["B4"].as<int>();

  for(int ITEM = 0; ITEM < (Payload["data"]["B5"].as<int>() - CurrentITEM[9]); ITEM++){
      mcpU7.writeRegister(MCP23017Register::GPIO_A, MotorB5);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_5_RVC(0x10,0x00);
          delay(20);
      }  
      CurrentITEM[9]  = Payload["data"]["B5"].as<int>();
  
  for(int ITEM = 0; ITEM < (Payload["data"]["C1"].as<int>() - CurrentITEM[10]); ITEM++){
      mcpU6.writeRegister(MCP23017Register::GPIO_A, MotorC1);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_C_RVC(0x02,0x00);
          delay(20);
      }    
      CurrentITEM[10] = Payload["data"]["C1"].as<int>();
  
  for(int ITEM = 0; ITEM < (Payload["data"]["C2"].as<int>() - CurrentITEM[11]); ITEM++){
      mcpU6.writeRegister(MCP23017Register::GPIO_A, MotorC2);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_C_RVC(0x08,0x00);
          delay(20);
      }
      CurrentITEM[11] = Payload["data"]["C2"].as<int>();
  
  for(int ITEM = 0; ITEM < (Payload["data"]["C3"].as<int>() - CurrentITEM[12]); ITEM++){
      mcpU6.writeRegister(MCP23017Register::GPIO_A, MotorC3);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_C_RVC(0x20,0x00);
          delay(20);
      }
      CurrentITEM[12] = Payload["data"]["C3"].as<int>();
  
  for(int ITEM = 0; ITEM < (Payload["data"]["C4"].as<int>() - CurrentITEM[13]); ITEM++){
      mcpU6.writeRegister(MCP23017Register::GPIO_A, MotorC4);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_C_RVC(0x80,0x00);
          delay(20);
      }  
      CurrentITEM[13] = Payload["data"]["C4"].as<int>();
  
  for(int ITEM = 0; ITEM < (Payload["data"]["C5"].as<int>() - CurrentITEM[14]); ITEM++){  
    mcpU7.writeRegister(MCP23017Register::GPIO_A, MotorC5);  //Reset port B
    for(int S=0; S < (200*Rotation); S++)
        LINE_5_RVC(0x0C,0x08);
        delay(20);
    }   
    CurrentITEM[14] = Payload["data"]["C5"].as<int>();
  
  for(int ITEM = 0; ITEM < (Payload["data"]["D1"].as<int>() - CurrentITEM[15]); ITEM++){
      mcpU6.writeRegister(MCP23017Register::GPIO_A, MotorD1);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_D_RVC(0x40,0x00);
          delay(20);
      }  
      CurrentITEM[15] = Payload["data"]["D1"].as<int>();
              
  for(int ITEM = 0; ITEM < (Payload["data"]["D2"].as<int>() - CurrentITEM[16]); ITEM++){
      mcpU6.writeRegister(MCP23017Register::GPIO_A, MotorD2);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_D_RVC(0x10,0x00);
          delay(20);
      }  
      CurrentITEM[16] = Payload["data"]["D2"].as<int>();

  for(int ITEM = 0; ITEM < (Payload["data"]["D3"].as<int>() - CurrentITEM[17]); ITEM++){
      mcpU6.writeRegister(MCP23017Register::GPIO_A, MotorD3);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_D_RVC(0x04,0x00);
          delay(20);
      }  
      CurrentITEM[17] = Payload["data"]["D3"].as<int>();
  
  for(int ITEM = 0; ITEM < (Payload["data"]["D4"].as<int>() - CurrentITEM[18]); ITEM++){
    mcpU6.writeRegister(MCP23017Register::GPIO_A, MotorD4);  //Reset port B
    for(int S=0; S < (200*Rotation); S++)
        LINE_D_RVC(0x01,0x00);
        delay(20);
    }  
    CurrentITEM[18] = Payload["data"]["D4"].as<int>();
  
  for(int ITEM = 0; ITEM < (Payload["data"]["D5"].as<int>() - CurrentITEM[19]); ITEM++){
    mcpU7.writeRegister(MCP23017Register::GPIO_A, MotorD5);  //Reset port B
    for(int S=0; S < (200*Rotation); S++)
        LINE_5_RVC(0x01,0x00);
        delay(20);
    }  
    CurrentITEM[19] = Payload["data"]["D5"].as<int>();


  for(int ITEM = 0; ITEM < (Payload["data"]["E1"].as<int>() - CurrentITEM[20]); ITEM++){
      mcpU7.writeRegister(MCP23017Register::GPIO_B, MotorE1);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_E_RVC(0x02,0x00);
          delay(20);
      }
      CurrentITEM[20] = Payload["data"]["E1"].as<int>();
  
  for(int ITEM = 0; ITEM < (Payload["data"]["E2"].as<int>() - CurrentITEM[21]); ITEM++){
      mcpU7.writeRegister(MCP23017Register::GPIO_B, MotorE2);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_E_RVC(0x08,0x00);
          delay(20);
      }
      CurrentITEM[21] = Payload["data"]["E2"].as<int>();
  
  for(int ITEM = 0; ITEM < (Payload["data"]["E3"].as<int>() - CurrentITEM[22]); ITEM++){
      mcpU7.writeRegister(MCP23017Register::GPIO_B, MotorE3);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_E_RVC(0x20,0x00);
          delay(20);
      }
      CurrentITEM[22] = Payload["data"]["E3"].as<int>();
  
  for(int ITEM = 0; ITEM < (Payload["data"]["E4"].as<int>() - CurrentITEM[23]); ITEM++){
      mcpU7.writeRegister(MCP23017Register::GPIO_B, MotorE4);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_E_RVC(0x80,0x00);
          delay(20);
      }
      CurrentITEM[23] = Payload["data"]["E4"].as<int>();

  for(int ITEM = 0; ITEM < (Payload["data"]["E5"].as<int>() - CurrentITEM[24]); ITEM++){
      mcpU7.writeRegister(MCP23017Register::GPIO_A, MotorE5);  //Reset port B
      for(int S=0; S < (200*Rotation); S++)
          LINE_X_RVC(0x02,0x00);
          delay(20);
      }   
      CurrentITEM[24] = Payload["data"]["E5"].as<int>();
  Write_update_ITEM();
  Serial.println(" :: All Motors back for new Refill data :: ");
  mcpU6.writeRegister(MCP23017Register::GPIO_A, 0);
  mcpU6.writeRegister(MCP23017Register::GPIO_B, 0); 
  mcpU7.writeRegister(MCP23017Register::GPIO_A, 0); 
  mcpU7.writeRegister(MCP23017Register::GPIO_B, 0);  
}


////////////////////////////////////////////
//    this function will Reconnect with MQTT Server 
void reconnect(){
   while (!client.connected()) {
     Serial.print("Attempting MQTT connection...");
     if (client.connect(Client_ID)){
        Serial.println("connected");
        client.subscribe(SUB_TP1);
        client.subscribe(SUB_TP2);
     } else{
          Serial.print("failed, rc=");
          Serial.print(client.state());
          Serial.println(" try again in 3 seconds");
          delay(3000);
     }
  }
}


void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // publish a message roughly every 10min for alive deive connection .
  if (millis() - lastMillis > 600000) {
      lastMillis = millis();
      if(client.publish("VM_NI/SE/HB","HeartBeat"))
          Serial.println("\n\tPublished ......");
     Read_update_ITEM(); 
  mcpU6.writeRegister(MCP23017Register::GPIO_A, 0);
  mcpU6.writeRegister(MCP23017Register::GPIO_B, 0); 
  mcpU7.writeRegister(MCP23017Register::GPIO_A, 0); 
  mcpU7.writeRegister(MCP23017Register::GPIO_B, 0);  
  }
  
  if(RCV_ORDER_FLAG){         //  it will check if we recived any new order
      Check_last_order();
      RCV_ORDER_FLAG = false;
  }

  if(RCV_NEW_ORDER){          //  it will work on new order
     Check_loc();
     Write_update_ITEM();
     RCV_NEW_ORDER = false;

    MOTOR_IN_D(1,18);    // open inner door

    delay(20);
    MOTOR_PUSH(1,93);    // pusing item out side 

     delay(200);
     MOTOR_PUSH(0,16);    // close bit 
     delay(20); 
     MOTOR_IN_D(0,18);    // closined the inner door 
     serverUpdate(G_OrderID);
     delay(20);
     MOTOR_OT_D(1,25);    // open outer door

     delay(20);
     MOTOR_PUSH(0,77);    // close all right

     delay(60000);        // waiting 2 min       
     MOTOR_OT_D(0,25);    // close outer door
     delay(10000);
      
  
  mcpU6.writeRegister(MCP23017Register::GPIO_A, 0);
  mcpU6.writeRegister(MCP23017Register::GPIO_B, 0); 
  mcpU7.writeRegister(MCP23017Register::GPIO_A, 0); 
  mcpU7.writeRegister(MCP23017Register::GPIO_B, 0);  
  }  
  
  if(RCV_REFIL_FLAG){
     Write_update_ITEM(); 
     RCV_REFIL_FLAG = false;
  }
 

  if(digitalRead(reBoot_PIN) == LOW){
    Serial.println("Wiping WiFi credentials from memory...");
    wipeEEPROM();
    Serial.println("\n\nDevice Fully Reset!\n\n");
    Serial.println("\n\n** Restart after 5 Sec ! **\n\n");
    delay(5500);
    ESP.restart();
    }

  mcpU6.writeRegister(MCP23017Register::GPIO_A, 0);
  mcpU6.writeRegister(MCP23017Register::GPIO_B, 0); 
  mcpU7.writeRegister(MCP23017Register::GPIO_A, 0); 
  mcpU7.writeRegister(MCP23017Register::GPIO_B, 0); 
}

////////////////////////////////////////////
//    This function push item in outer box
void MOTOR_PUSH(bool Dir, int Rot){             /// Lift will move up from GND to A locatiion
  if(Dir)
    Serial.println(" :: Pushing item Righ Side :: ");
  else  
    Serial.println(" :: Push pad moving Left SIde :: ");    
    mcpU7.writeRegister(MCP23017Register::GPIO_A, Motor_PUSH_0);  //Reset port B
    for(int S=0; S < (200*Rot); S++){
      if(Dir){
        LINE_X_FRD(0x08,0x00); 
      }else
        LINE_X_RVC(0x0C,0x04); 
    }
    delay(20);
}

////////////////////////////////////////////
// This function will open inner door 
void MOTOR_IN_D(bool Dir, int Rot){   
  if(Dir)
    Serial.println(" :: Opening the INNER DOOR :: ");
  else  
    Serial.println(" :: Closing the INNER DOOR :: ");
    mcpU7.writeRegister(MCP23017Register::GPIO_A, Motor_Door_1);  //Reset port B
    for(int S=0; S < (200*Rot); S++){
      if(Dir)
        LINE_X_FRD(0x20,0x00);
      else
        LINE_X_RVC(0x30,0x10); 
    }
    delay(20);
}

////////////////////////////////////////////
// This function will open outer door 
void MOTOR_OT_D(bool Dir, int Rot){             /// Lift will move up from GND to A locatiion
  if(Dir)
    Serial.println(" :: Opening the OUTER DOOR :: ");
  else  
    Serial.println(" :: Closing the OUTER DOOR :: ");    
    mcpU7.writeRegister(MCP23017Register::GPIO_A, Motor_Door_2);  //Reset port B
    for(int S=0; S < (200*Rot); S++){
      if(Dir){
        digitalWrite(Relay_P, HIGH);    
        LINE_X_FRD(0x80,0x00); 
      }else{
        digitalWrite(Relay_P, LOW);    
        LINE_X_RVC(0xC0,0x40);
        }
    }
    delay(20);
}


void Check_loc(){
  Serial.println(" :: We Get new ITEM lift will up now :: ");

  for (int aa =0; aa <4; aa++){
      if(LOC_E){
        Serial.println(" :: Going location E :: ");  
        for (int zz=0;zz<20000;zz++){
            int Delay = (zz < 5 )? 2999 : 820;
            Lift_AT_A(Delay);
            Serial.println(zz);
            Serial.println(digitalRead(Location_E));
            if(! digitalRead(Location_E))
              break;
        }
        Motor_E();  delay(100);
      }
      if(LOC_D){
        Serial.println(" :: Going location D :: ");  
        for (int zz=0;zz<42000;zz++){
            int Delay = (zz < 5 )? 2999 : 820;
            Lift_AT_A(Delay);
            Serial.println(zz);
            Serial.println(digitalRead(Location_D));
            if(! digitalRead(Location_D))
              break;      
          }
        Motor_D();  delay(100);
      }
      if(LOC_C){ 
        Serial.println(" :: Going location C :: ");
        for (int zz=0;zz<64000;zz++){
            int Delay = (zz < 5 )? 2999 : 820;
            Lift_AT_A(Delay);
            Serial.println(zz);
            Serial.println(digitalRead(Location_C));
            if(! digitalRead(Location_C))
              break;      
          }
        Motor_C();  delay(100);
      }
      if(LOC_B){
        Serial.println(" :: Going location B :: ");
        for (int zz=0;zz<86000;zz++){
            int Delay = (zz < 5 )? 2999 : 820;
            Lift_AT_A(Delay);
            Serial.println(zz);
            Serial.println(digitalRead(Location_B));
            if(! digitalRead(Location_B))
              break;      
          }
        Motor_B(); delay(100);
      }
      if(LOC_A){
        Serial.println(" :: Going location A :: ");
        for (int zz=0;zz<99000;zz++){
            int Delay = (zz < 5 )? 2999 : 820;
            Lift_AT_A(Delay);
            Serial.println(zz);
            Serial.println(digitalRead(liftMAX));
            if(! digitalRead(liftMAX))
              break;      
          } 
        Motor_A(); delay(100);
      }
      break;
  }
  Serial.println(" :: Going location GND :: ");
  for (long zz=0;zz<99000 ;zz++){
    int Delay = (zz < 5 )? 2999 : 820;
    Lift_ONE_DOWN(Delay);
    if(!digitalRead(liftMIN))        // if bymistak lift go on max low will go bit down 
        break;
  }
  mcpU6.writeRegister(MCP23017Register::GPIO_A, 0);
  mcpU6.writeRegister(MCP23017Register::GPIO_B, 0); 
  mcpU7.writeRegister(MCP23017Register::GPIO_A, 0); 
  mcpU7.writeRegister(MCP23017Register::GPIO_B, 0); 
}

void Motor_A(void){

  Serial.println(" :: LIFT at LOC_A :: ");
  for(int aa=0; aa<4; aa++){      // lift moved at location A from Zero
      if(LOC_A){
          if(G_Loc[aa] == "A1"){
              Serial.println("\t I got the ..A location with 1 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  mcpU6.writeRegister(MCP23017Register::GPIO_B, MotorA1);  //Reset port B
                  CurrentITEM[0] =  CurrentITEM[0] -1;
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_A_FRD(0x03,0x01);
                  delay(20);
              }
          }else if(G_Loc[aa] == "A2"){
              Serial.println("\t I got the ..A location with 2 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  mcpU6.writeRegister(MCP23017Register::GPIO_B, MotorA2);  //Reset port B
                  CurrentITEM[1] =  CurrentITEM[1] -1;
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_A_FRD(0x0C,0x04);
                  delay(20);
              }          
          }else if(G_Loc[aa] == "A3"){
              Serial.println("\t I got the ..A location with 3 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[2] =  CurrentITEM[2] -1;
                  mcpU6.writeRegister(MCP23017Register::GPIO_B, MotorA3);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_A_FRD(0x30,0x10);
                  delay(20);
              }   
          }else if(G_Loc[aa] == "A4"){
              Serial.println("\t I got the ..A location with 4 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[3] =  CurrentITEM[3] -1;
                  mcpU6.writeRegister(MCP23017Register::GPIO_B, MotorA4);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_A_FRD(0xC0,0x40);
                  delay(20);
              } 
          }else if(G_Loc[aa] == "A5"){
              Serial.println("\t I got the ..A location with 5 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[4] =  CurrentITEM[4] -1;
                  mcpU7.writeRegister(MCP23017Register::GPIO_A, MotorA5);  //Reset port A
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_5_FRD(0x40,0x00);
                  delay(20);
              } 
          }
      }
    break;
  }
}

void Motor_B(void){

  Serial.println(" :: LIFT at LOC_B :: ");   
  for (int aa=0; aa<4; aa++){  // lift moved at location B from A
      if(LOC_B){    
          if(G_Loc[aa] == "B1"){
              Serial.println("\t I got the ..B location with 1 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[5] =  CurrentITEM[5] -1;
                  mcpU6.writeRegister(MCP23017Register::GPIO_B, MotorB1);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_B_FRD(0xC0,0x80);
                  delay(20);
              }             
          }else if(G_Loc[aa] == "B2"){
              Serial.println("\t I got the ..B location with 2 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[6] =  CurrentITEM[6] -1;
                  mcpU6.writeRegister(MCP23017Register::GPIO_B, MotorB2);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_B_FRD(0x30,0x20);
                  delay(20);
              }    
          }else if(G_Loc[aa] == "B3"){
              Serial.println("\t I got the ..B location with 3 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[7] =  CurrentITEM[7] -1;
                  mcpU6.writeRegister(MCP23017Register::GPIO_B, MotorB3);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_B_FRD(0x0C,0x08);
                  delay(20);
              }    
          }else if(G_Loc[aa] == "B4"){
              Serial.println("\t I got the ..B location with 4 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[8] =  CurrentITEM[8] -1;
                  mcpU6.writeRegister(MCP23017Register::GPIO_B, MotorB4);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_B_FRD(0x03,0x02);
                  delay(20);
              }    
          }else if(G_Loc[aa] == "B5"){
              Serial.println("\t I got the ..B location with 5 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[9] =  CurrentITEM[9] -1;
                  mcpU7.writeRegister(MCP23017Register::GPIO_A, MotorB5);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_5_FRD(0x30,0x20);
                  delay(20);
              }    
          }
      }
    break;
  }
}

void Motor_C(void){

  Serial.println(" :: LIFT at LOC_C :: ");   
  for (int aa=0; aa<4; aa++){  // lift moved at location C from B
      if(LOC_C){    
          if(G_Loc[aa] == "C1"){
              Serial.println("\t I got the ..C location with 1 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[10] =  CurrentITEM[10] -1;
                  mcpU6.writeRegister(MCP23017Register::GPIO_A, MotorC1);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_C_FRD(0x03,0x01);
                  delay(20);
              }              
          }else if(G_Loc[aa] == "C2"){
              Serial.println("\t I got the ..c location with 2 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[11] =  CurrentITEM[11] -1;
                  mcpU6.writeRegister(MCP23017Register::GPIO_A, MotorC2);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_C_FRD(0x0C,0x04);
                  delay(20);
              }              
          }else if(G_Loc[aa] == "C3"){
              Serial.println("\t I got the ..C location with 3 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[12] =  CurrentITEM[12] -1;
                  mcpU6.writeRegister(MCP23017Register::GPIO_A, MotorC3);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_C_FRD(0x30,0x10);
                  delay(20);
              }              
          }else if(G_Loc[aa] == "C4"){
              Serial.println("\t I got the ..C location with 4 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[13] =  CurrentITEM[13] -1;
                  mcpU6.writeRegister(MCP23017Register::GPIO_A, MotorC4);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_C_FRD(0xC0,0x40);
                  delay(20);
              }              
          }else if(G_Loc[aa] == "C5"){
              Serial.println("\t I got the ..C location with 5 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[14] =  CurrentITEM[14] -1;
                  mcpU7.writeRegister(MCP23017Register::GPIO_A, MotorC5);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_5_FRD(0x04,0x00);
                  delay(20);
              }              
          }
      }
    break;
  }
}

void Motor_D(void){

  Serial.println(" :: LIFT at LOC_D :: ");
  for (int aa=0; aa<4; aa++){  // lift moved at location D from C
      if(LOC_D){
          if(G_Loc[aa] == "D1"){
              Serial.println("\t I got the ..D location with 1 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[15] =  CurrentITEM[15] -1;
                  mcpU6.writeRegister(MCP23017Register::GPIO_A, MotorD1);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_D_FRD(0xC0,0x80);
                  delay(20);
              }    
          }else if(G_Loc[aa] == "D2"){
              Serial.println("\t I got the ..D location with 2 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[16] =  CurrentITEM[16] -1;
                  mcpU6.writeRegister(MCP23017Register::GPIO_A, MotorD2);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_D_FRD(0x30,0x20);
                  delay(20);
              }    
          }else if(G_Loc[aa] == "D3"){
              Serial.println("\t I got the ..D location with 3 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[17] =  CurrentITEM[17] -1;
                  mcpU6.writeRegister(MCP23017Register::GPIO_A, MotorD3);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_D_FRD(0x0c,0x08);
                  delay(20);
              }                  
          }else if(G_Loc[aa] == "D4"){
              Serial.println("\t I got the ..D location with 4 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[18] =  CurrentITEM[18] -1;
                  mcpU6.writeRegister(MCP23017Register::GPIO_A, MotorD4);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_D_FRD(0x03,0x02);
                  delay(20);
              }    
          }else if(G_Loc[aa] == "D5"){
              Serial.println("\t I got the ..D location with 5 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[19] =  CurrentITEM[19] -1;
                  mcpU7.writeRegister(MCP23017Register::GPIO_A, MotorD5);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_5_FRD(0x03,0x02);
                  delay(20);
              }    
          }
      }  
    break;  
  }
}

void Motor_E(void){

  Serial.println(" :: LIFT at LOC_E :: ");
  for (int aa=0; aa<4; aa++){  // lift moved at location E from D
     if(LOC_E){
          if(G_Loc[aa] == "E1"){
              Serial.println("\t I got the ..E location with 1 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[20] =  CurrentITEM[20] -1;
                  mcpU7.writeRegister(MCP23017Register::GPIO_B, MotorE1);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_E_FRD(0x03,0x01);
                  delay(20);
              }
          }else if(G_Loc[aa] == "E2"){
              Serial.println("\t I got the ..E location with 2 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[21] =  CurrentITEM[21] -1;
                  mcpU7.writeRegister(MCP23017Register::GPIO_B, MotorE2);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_E_RVC(0x0C,0x04);
                  delay(20);
              }
          }else if(G_Loc[aa] == "E3"){
              Serial.println("\t I got the ..E location with 3 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[22] =  CurrentITEM[22] -1;
                  mcpU7.writeRegister(MCP23017Register::GPIO_B, MotorE3);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_E_FRD(0x30,0x10);
                  delay(20);
              }
          }else if(G_Loc[aa] == "E4"){
              Serial.println("\t I got the ..E location with 4 Pos and ");  Serial.print(G_Qty[aa]);  Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[23] =  CurrentITEM[23] -1;
                  mcpU7.writeRegister(MCP23017Register::GPIO_B, MotorE4);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_E_FRD(0xC0,0x40);
                  delay(20);
              }          
          }else if(G_Loc[aa] == "E5"){
              Serial.println("\t I got the ..E location with 5 Pos and ");  Serial.print(G_Qty[aa]); Serial.println(" Item");
              for(int ITEM = 0; ITEM<G_Qty[aa]; ITEM++){
                  CurrentITEM[24] =  CurrentITEM[24] -1;
                  mcpU7.writeRegister(MCP23017Register::GPIO_A, MotorE5);  //Reset port B
                  for(int S=0; S < (200*Rotation); S++)
                      LINE_X_FRD(0x03,0x01);
                  delay(20);
              }          
          }
      } 
    break;
  }
}

////////////////////////////////////////////
// This function will lift up at location A
void Lift_AT_A(int Rq_delay){             /// Lift will move up from GND to A locatiion
  currentMicros = micros();
  if(currentMicros - previousMicros >= Rq_delay){
    previousMicros = currentMicros;
    mcpU5.writeRegister(MCP23017Register::GPIO_A, 0x40);  //Reset port B
    delayMicroseconds(100); //Set Value
    mcpU5.writeRegister(MCP23017Register::GPIO_A, 0x00);  //Reset port B
  } 
}

////////////////////////////////////////////
// This function will send lift to one step down 
void Lift_ONE_DOWN(int Rq_delay){
  currentMicros = micros();
  if(currentMicros - previousMicros >= Rq_delay){
    previousMicros = currentMicros;
    mcpU5.writeRegister(MCP23017Register::GPIO_A, 0xC0);  //Reset port B
    delayMicroseconds(100); //Set Value
    mcpU5.writeRegister(MCP23017Register::GPIO_A, 0x80);  //Reset port B
  } 
}

////////////////////////////////////////////
// This function will check if we are receiving any order twis or not 
void Check_last_order(){
  Serial.println(" :: Now Checking If we get Duplicate Order Or Not :: ");
      LASTORDER = EEPROM.read(EE_LAST_ADD+0)*10000 + EEPROM.read(EE_LAST_ADD+1)*100 + EEPROM.read(EE_LAST_ADD+2);
      Serial.print("LASTORDER > ");Serial.println(LASTORDER);
      Serial.print("G_OrderID > ");Serial.println(G_OrderID);
      delay(2000);
      if(LASTORDER == G_OrderID){
          Serial.println("We get the same order");
          RCV_NEW_ORDER = false;
      }else {
          Serial.println("New order");
          RCV_NEW_ORDER = true; 
          EEPROM.write(EE_LAST_ADD+0, G_OrderID/10000);
          EEPROM.write(EE_LAST_ADD+1, (G_OrderID%10000)/100);
          EEPROM.write(EE_LAST_ADD+2, (G_OrderID%10000)%100);  
          EEPROM.commit();     
      }
}

////////////////////////////////////////////
// will write all status in EEPROM
void Write_update_ITEM(){  
  for(int ss=0; ss<25; ss++)
      EEPROM.write(EE_ITEM_ADD+ss, CurrentITEM[ss]);  
  EEPROM.commit();

  Serial.println(" :: New Value from CurrentITEM update in EEPROM :: ");   
}

////////////////////////////////////////////
// will write ZERO at all location and reset 
void AllReset(){  
  for(int ss=0; ss<100; ss++)
      EEPROM.write(EE_ITEM_ADD+ss, 0);
      EEPROM.commit();   
}

////////////////////////////////////////////
// This will get all current data from EEPROM
void Read_update_ITEM(){
  Serial.println(" :: RIght Now we have Item :: ");  
  for(int ss=0; ss<25; ss++)
    CurrentITEM[ss] =  EEPROM.read(EE_ITEM_ADD+ss); 
    delay(3000);
    for(int ss=0; ss<25; ss++)
    Serial.println(CurrentITEM[ss]);
    Serial.println("----------------");  
}

////////////////////////////////////////////
// This will clear all saved data from EEPROM
void wipeEEPROM(){
  for(int i=199;i<299;i++){
    EEPROM.writeByte(i,0);
  }
  EEPROM.commit();
  Serial.println("\t *** WiFi Info Cleaned ***");  
}

////////////////////////////////////////////
// This will update on server Task done
void serverUpdate(int ID){
  for(int SS =0; SS<2; SS++){
    if((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
      Serial.println(" :: WiFi COnnected now update status on Server :: ");
      HTTPClient http;
      String URL = "https://brownbutter.nirmalinnovations.com/wp-json/wc/v3/orders/";
      String postData = "status=completed";

      http.begin(URL +String(ID));
      http.addHeader("accept","application/json");
      http.addHeader("authorization","Basic YWRtaW46eWV3dl5FSVVvbCNsT1pkTTJJb2NRXlgh");
      http.addHeader("Content-Type","application/x-www-form-urlencoded");
      
      int httpCode = http.POST(postData);         
      if(httpCode == 200) { //Check for the returning code
          String payload = http.getString();
          Serial.println(httpCode);
          Serial.println("....................");
          Serial.println(payload);
          Serial.println("....................");
          break;
      }else {
          Serial.println("Error on HTTP request");
          continue;
      }
      http.end(); //Free the resources
      delay(200);
    }else
      setup_wifi();
  }
}