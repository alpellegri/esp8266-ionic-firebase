#!/bin/bash

file=../.pio/build/featheresp32/firmware.bin
cp $file ESP32firmware.bin
printf $(md5sum ESP32firmware.bin) > ESP32firmware.md5