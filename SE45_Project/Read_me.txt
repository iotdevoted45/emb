
1. when power up YELLOW blink mean device is in AP(HotSpot) mode.
    > now connect your mobile with ESP32 wifi "#_NIMESH INNOVATIONS_#" witch have password  "123four567".
    > if connected that open URL of 192.168.4.1 and give your router SSID and PASS in the page and save.
    > if it connected to router than YELLOW will stop to blink 

2. Now if RED is blinking mean Wifi is not connected or wrong ID PASS you give 
        press reboot button for 10 Sec and again point 1 follow
    > or else Green if glow mean Wifi and MQTT is connected
    >   "VM_NI/SE/Order" : will give you the Order JSON from MQTT 
    >   "VM_NI/SE/Refilling" : Will give you the Refill JSON from MQTT
    > or else YELLOW or RED is blinking mean wifi connected but MQTT not connecting 

3. Now I will reset both door and life at Init location by liftSetup, CLose_D1 and Close_D2

4. If I will get reBoot_PIN pin LOW that will reboot system and start from point 1.
    > Else I can get order JSON or refill JSON
        > if I get OrderJSON
        > I will check the order is duplicate or not  if it is just pass else
        > I will check the order came for X location and lift will move as per that 
        > when lift will on GND location .. inside door will open .. Push all item out side and YELLOW LED will blink.
            than move back bit and inside door will close.
        > After that outer door open and green light will glow and push motor will go full back 
        > 1Min latter the outer door will close and RED LED will blink
    > and If I get refillJSON as per received number I will move back the motors 
