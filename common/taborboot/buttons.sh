#!/bin/bash

echo Copying start up images to SD card $1

sudo dd of=$1 if=logo.bmp bs=512 seek=1808

sudo dd of=$1 if=X5000_Boot01.bmp bs=512 seek=2048
sudo dd of=$1 if=X5000_Boot02.bmp bs=512 seek=3072
sudo dd of=$1 if=X5000_Boot03.bmp bs=512 seek=4096
sudo dd of=$1 if=X5000_Boot04.bmp bs=512 seek=5120
sudo dd of=$1 if=X5000_Boot05.bmp bs=512 seek=6144
sudo dd of=$1 if=X5000_Boot06.bmp bs=512 seek=7168
sudo dd of=$1 if=X5000_Boot07.bmp bs=512 seek=8192
sudo dd of=$1 if=X5000_Boot08.bmp bs=512 seek=9216
sudo dd of=$1 if=X5000_Boot09.bmp bs=512 seek=10240
sudo dd of=$1 if=X5000_Boot10.bmp bs=512 seek=11264

