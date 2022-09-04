idf_component_register(SRCS "main.c" "basicInit.c" "commondef.c" "fileOperations.c" "i2c_peri.c" "modbus.c" "mqtt.c" "msg_processor.c" "ota.c" "rtc_time.c" "wifiConnectivity.c"
                    INCLUDE_DIRS "." 
                    EMBED_TXTFILES s3all.pem)
                    
#idf_component_register(SRCS "basicInit.c" "commondef.c" "fileOperations.c" "i2c_peri.c" "modbus.c" "mqtt.c" "msg_processor.c" "ota.c" "rtc_time.c" "user_main.c" "wifiConnectivity.c"
#                    INCLUDE_DIRS "." 
#                    EMBED_TXTFILES s3all.pem)

#spiffs_create_partition_image(storage ../spiffs_image FLASH_IN_PROJECT)
