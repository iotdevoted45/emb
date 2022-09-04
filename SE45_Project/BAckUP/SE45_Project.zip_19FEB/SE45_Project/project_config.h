
#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

#include <WiFi.h>
#include <EEPROM.h> 
#include <ArduinoJson.h>
#include <PubSubClient.h>

// Information for MQTT Broker and Wifi Connection 
// Replace the next variables with your SSID/Password combination
#define Client_ID "Suthar_Electronics"
const char* ssid     = "#_NIMESH INNOVATIONS_#";
const char* pass     = "123four567";
const char* Broker = "broker.hivemq.com";
const  int  M_port = 1883;
const char* SUB_TP1 = "VM_NI/SE/Order";
const char* SUB_TP2 = "VM_NI/SE/Refilling";

// Define stepper motor connections:
char Rotation = 12;

// The MCP have the Address from EXT PIN
#define MCP23017_U2 0x20    // U2 >> 000
#define MCP23017_U3 0x24    // U3 >> 100
#define MCP23017_U4 0x22    // U4 >> 010
#define MCP23017_U5 0x21    // U5 >> 001
#define MCP23017_U6 0x23    // U6 >> 011
#define MCP23017_U7 0x26    // U7 >> 110

#define MotorA1 0x01    // PCB_M01    MOTOER_POWER from U6 PORT B1
#define MotorA2 0x02    // PCB_M02    MOTOER_POWER from U6 PORT B2
#define MotorA3 0x04    // PCB_M03    MOTOER_POWER from U6 PORT B3
#define MotorA4 0x08    // PCB_M04    MOTOER_POWER from U6 PORT B4
#define MotorB1 0x10    // PCB_M05    MOTOER_POWER from U6 PORT B5
#define MotorB2 0x20    // PCB_M06    MOTOER_POWER from U6 PORT B6
#define MotorB3 0x40    // PCB_M07    MOTOER_POWER from U6 PORT B7
#define MotorB4 0x80    // PCB_M08    MOTOER_POWER from U6 PORT B8
#define MotorC1 0x01    // PCB_M09    MOTOER_POWER from U6 PORT A1
#define MotorC2 0x02    // PCB_M10    MOTOER_POWER from U6 PORT A2
#define MotorC3 0x04    // PCB_M11    MOTOER_POWER from U6 PORT A3
#define MotorC4 0x08    // PCB_M12    MOTOER_POWER from U6 PORT A4
#define MotorD1 0x10    // PCB_M13    MOTOER_POWER from U6 PORT A5
#define MotorD2 0x20    // PCB_M14    MOTOER_POWER from U6 PORT A6
#define MotorD3 0x40    // PCB_M15    MOTOER_POWER from U6 PORT A7
#define MotorD4 0x80    // PCB_M16    MOTOER_POWER from U6 PORT A8
#define MotorE1 0x08    // PCB_M17    MOTOER_POWER from U7 PORT B4
#define MotorE2 0x04    // PCB_M18    MOTOER_POWER from U7 PORT B3
#define MotorE3 0x02    // PCB_M19    MOTOER_POWER from U7 PORT B2
#define MotorE4 0x01    // PCB_M20    MOTOER_POWER from U7 PORT B1
#define MotorA5 0x01    // PCB_M21    MOTOER_POWER from U7 PORT A1
#define MotorB5 0x02    // PCB_M22    MOTOER_POWER from U7 PORT A2
#define MotorC5 0x04    // PCB_M23    MOTOER_POWER from U7 PORT A3
#define MotorD5 0x08    // PCB_M24    MOTOER_POWER from U7 PORT A4
#define MotorE5 0x10    // PCB_M25    MOTOER_POWER from U7 PORT A5
#define Motor_PUSH_0 0x20    // PCB_M26    MOTOER_POWER from U7 PORT A6
#define Motor_Door_1 0x40    // PCB_M27    MOTOER_POWER from U7 PORT A7
#define Motor_Door_2 0x80    // PCB_M28    MOTOER_POWER from U7 PORT A8



#define A1_FRD  {0x02,0x00}
#define A1_RVC  {0x03,0x01}
#define A2_FRD  {0x08,0x00}
#define A2_RVC  {0x0C,0x04}   // Motor connection U2 PORT_B
#define A3_FRD  {0x20,0x00}
#define A3_RVC  {0x30,0x10}
#define A4_FRD  {0x80,0x00}
#define A4_RVC  {0xC0,0x40}
#define A5_FRD  {0xC0,0x80}
#define A5_RVC  {0x40,0x00}

#define B1_FRD  {0xC0,0x80}
#define B1_RVC  {0x40,0x00}
#define B2_FRD  {0x30,0x20}
#define B2_RVC  {0x10,0x00}
#define B3_FRD  {0x0C,0x08}   // Motor connection U2 PORT_A
#define B3_RVC  {0x04,0x00}
#define B4_FRD  {0x03,0x02}
#define B4_RVC  {0x01,0x00}
#define B5_FRD  {0x30,0x20}
#define B5_RVC  {0x10,0x00}

#define C1_FRD  {0x02,0x00}
#define C1_RVC  {0x03,0x01}
#define C2_FRD  {0x08,0x00}
#define C2_RVC  {0x0C,0x04}   // Motor connection U3 PORT_B
#define C3_FRD  {0x20,0x00}
#define C3_RVC  {0x30,0x10}
#define C4_FRD  {0x80,0x00}
#define C4_RVC  {0xC0,0x40}
#define C5_FRD  {0x0C,0x08}
#define C5_RVC  {0x04,0x00}

#define D1_FRD  {0xC0,0x80}
#define D1_RVC  {0x40,0x00}
#define D2_FRD  {0x30,0x20}
#define D2_RVC  {0x10,0x00}
#define D3_FRD  {0x0C,0x08}   // Motor connection U3 PORT_A
#define D3_RVC  {0x04,0x00}
#define D4_FRD  {0x03,0x02}
#define D4_RVC  {0x01,0x00}
#define D5_FRD  {0x03,0x02}
#define D5_RVC  {0x01,0x00}

#define E1_FRD  {0x02,0x00}
#define E1_RVC  {0x03,0x01}
#define E2_FRD  {0x08,0x00}
#define E2_RVC  {0x0C,0x04}   // Motor connection U4 PORT_B
#define E3_FRD  {0x20,0x00}
#define E3_RVC  {0x30,0x10}
#define E4_FRD  {0x80,0x00}
#define E4_RVC  {0xC0,0x40}
#define E5_FRD  {0x02,0x00}
#define E5_RVC  {0x03,0x01}

#define push0_FRD  {0x08,0x00}
#define push0_RVC  {0x0C,0x04}   // Motor connection U5 PORT_B
#define door1_FRD  {0x20,0x00}
#define door1_RVC  {0x30,0x10}
#define door2_FRD  {0x80,0x00}
#define door2_RVC  {0xC0,0x40}
#endif  