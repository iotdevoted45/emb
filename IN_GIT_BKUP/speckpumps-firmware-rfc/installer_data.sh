#!/bin/sh

mkdir -p binary_files
cp ./build/bootloader/bootloader.bin ./binary_files/.
cp ./build/partition_table/partition-table.bin ./binary_files/.
cp ./build/ota_data_initial.bin ./binary_files/.
cp ./build/RFB_0_1_0.bin ./binary_files/.
cp ./build/storage.bin ./binary_files/.
cp binary_map.txt ./binary_files/.

cp ./build/RFB_0_1_0.bin ./binary_files/OTA_RFB_0_1_0.bin
cp spiffsgen.py ./binary_files/.
