#!/bin/sh

ESPAT=`pwd`
cd ..
ESPSDK=`pwd`
cd $ESPAT

#export ESPSDK=$HOME/apps/esp/sdk
#export ESPSDK=$HOME/Projects/esp8266-wiki/esp_iot_sdk_v1.0.0
#export ESPAT=$ESPSDK/canwii
export PATH=/opt/esp/esptool/:$PATH
export PATH=$HOME/apps/esp/esptool/:$PATH

echo "ESPSDK:$ESPSDK"
echo "ESP AT:$ESPAT"

canwii_app="canwii_app_0x00000.bin"

cd $ESPAT
cd .output/eagle/debug/image
esptool -eo eagle.app.v6.out -bo eagle.app.v6.flash.bin -bs .text -bs .data -bs .rodata -bc -ec
xtensa-lx106-elf-objcopy --only-section .irom0.text -O binary eagle.app.v6.out eagle.app.v6.irom0text.bin
cp eagle.app.v6.flash.bin $ESPSDK/bin/$canwii_app
cp eagle.app.v6.irom0text.bin $ESPSDK/bin/esp_sdk_0x40000.bin

if [ -f $ESPSDK/bin/$canwii_app ] ;then
   echo "CANWII application at: $ESPSDK/bin/$canwii_app"
fi

if [ -f $ESPSDK/bin/esp_sdk_0x00000.bin ] ; then
   echo "SDK at:$ESPSDK/bin/esp_sdk_0x00000.bin"
fi


