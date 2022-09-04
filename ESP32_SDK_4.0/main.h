#ifndef IOT_MAIN_H
#define IOT_MAIN_H
 #include "stdio.h"
 //for getting real time from NTP server
#include "time.h"

void setConnectionStatusChangedCallback(String PayLoad);
void setDeviceChangedCallback(String PayLoad);
void setModuleRecievedCallback(String PayLoad);
String GetTime();

#endif  // IOT_MAIN_H
