#!/bin/bash

echo Copying start up images to SD card $1

sudo dd of=$1 if=logo.bmp bs=512 seek=1808

sudo dd of=$1 if=A1222_Boot_v1001.bmp bs=512 seek=65536
sudo dd of=$1 if=A1222_Boot_v1002.bmp bs=512 seek=67584
sudo dd of=$1 if=A1222_Boot_v1003.bmp bs=512 seek=69632
sudo dd of=$1 if=A1222_Boot_v1004.bmp bs=512 seek=71680
sudo dd of=$1 if=A1222_Boot_v1005.bmp bs=512 seek=73728
sudo dd of=$1 if=A1222_Boot_v1006.bmp bs=512 seek=75776
sudo dd of=$1 if=A1222_Boot_v1007.bmp bs=512 seek=77824
sudo dd of=$1 if=A1222_Boot_v1008.bmp bs=512 seek=79872
sudo dd of=$1 if=A1222_Boot_v1009.bmp bs=512 seek=81920
sudo dd of=$1 if=A1222_Boot_v1010.bmp bs=512 seek=83968
sudo dd of=$1 if=A1222_Boot_v1011.bmp bs=512 seek=86016
sudo dd of=$1 if=A1222_Boot_v1012.bmp bs=512 seek=88064
sudo dd of=$1 if=A1222_Boot_v1013.bmp bs=512 seek=90112
sudo dd of=$1 if=A1222_Boot_v1014.bmp bs=512 seek=92160
sudo dd of=$1 if=A1222_Boot_v1015.bmp bs=512 seek=94208
sudo dd of=$1 if=A1222_Boot_v1016.bmp bs=512 seek=96256
sudo dd of=$1 if=A1222_Boot_v1017.bmp bs=512 seek=98304
sudo dd of=$1 if=A1222_Boot_v1018.bmp bs=512 seek=100352
sudo dd of=$1 if=A1222_Boot_v1019.bmp bs=512 seek=102400
sudo dd of=$1 if=A1222_Boot_v1020.bmp bs=512 seek=104448
sudo dd of=$1 if=A1222_Boot_v1021.bmp bs=512 seek=106496
sudo dd of=$1 if=A1222_Boot_v1022.bmp bs=512 seek=108544
sudo dd of=$1 if=A1222_Boot_v1023.bmp bs=512 seek=110592
sudo dd of=$1 if=A1222_Boot_v1024.bmp bs=512 seek=112640
sudo dd of=$1 if=A1222_Boot_v1025.bmp bs=512 seek=114688
sudo dd of=$1 if=A1222_Boot_v1026.bmp bs=512 seek=116736
sudo dd of=$1 if=A1222_Boot_v1027.bmp bs=512 seek=118784
sudo dd of=$1 if=A1222_Boot_v1028.bmp bs=512 seek=120832
sudo dd of=$1 if=A1222_Boot_v1029.bmp bs=512 seek=122880
sudo dd of=$1 if=A1222_Boot_v1030.bmp bs=512 seek=124928
sudo dd of=$1 if=A1222_Boot_v1031.bmp bs=512 seek=126976
sudo dd of=$1 if=A1222_Boot_v1032.bmp bs=512 seek=129024
sudo dd of=$1 if=A1222_Boot_v1033.bmp bs=512 seek=131072
sudo dd of=$1 if=A1222_Boot_v1034.bmp bs=512 seek=133120
sudo dd of=$1 if=A1222_Boot_v1035.bmp bs=512 seek=135168
sudo dd of=$1 if=A1222_Boot_v1036.bmp bs=512 seek=137216

