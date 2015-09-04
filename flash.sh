#!/bin/bash
ESPAT=`pwd`
cd ..
ESPSDK=`pwd`
cd $ESPAT


#export PATH=$HOME/apps/esp/esptool/:$PATH

#export ESPSDK=$HOME/apps/esp/sdk

cd $ESPSDK/bin

PORT=$1
WHICH=$2


ATAPP=canwii_app_0x00000.bin
ESPSDK=esp_sdk_0x40000.bin
BLANK=blank.bin
DEFAULTBIN=esp_init_data_default.bin

if [ "$WHICH" == "both" ];then

   echo "Cleaning the flash and flashing SDK and APP"
   esptool.py --port $PORT -b 115200 write_flash 0x00000 $BLANK 0x01000 $BLANK 0x40000 $BLANK 0x7c000 $BLANK 0x7e000 $BLANK 0x3D000 $BLANK 0x7e000 $DEFAULTBIN 0x40000 $ESPSDK 0x00000 $ATAPP 
   
   exit 0
fi

esptool.py --port $PORT -b 115200 write_flash 0x3D000 $BLANK 0x00000 $ATAPP 
cd $ESPAT
