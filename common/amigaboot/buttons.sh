#!/bin/bash

echo Copying button images to SD card $1

sudo dd of=$1 if=background.bmp bs=512 seek=1280

sudo dd of=$1 if=button1.bmp bs=512 seek=1296
sudo dd of=$1 if=button2.bmp bs=512 seek=1312
sudo dd of=$1 if=button3.bmp bs=512 seek=1328
sudo dd of=$1 if=button4.bmp bs=512 seek=1344
sudo dd of=$1 if=button5.bmp bs=512 seek=1360
sudo dd of=$1 if=button6.bmp bs=512 seek=1376
sudo dd of=$1 if=button7.bmp bs=512 seek=1392
sudo dd of=$1 if=button8.bmp bs=512 seek=1408
sudo dd of=$1 if=button9.bmp bs=512 seek=1424
sudo dd of=$1 if=button10.bmp bs=512 seek=1440
sudo dd of=$1 if=button11.bmp bs=512 seek=1456

sudo dd of=$1 if=button1rev.bmp bs=512 seek=1472
sudo dd of=$1 if=button2rev.bmp bs=512 seek=1488
sudo dd of=$1 if=button3rev.bmp bs=512 seek=1504
sudo dd of=$1 if=button4rev.bmp bs=512 seek=1520
sudo dd of=$1 if=button5rev.bmp bs=512 seek=1536
sudo dd of=$1 if=button6rev.bmp bs=512 seek=1552
sudo dd of=$1 if=button7rev.bmp bs=512 seek=1568
sudo dd of=$1 if=button8rev.bmp bs=512 seek=1584
sudo dd of=$1 if=button9rev.bmp bs=512 seek=1600
sudo dd of=$1 if=button10rev.bmp bs=512 seek=1616
sudo dd of=$1 if=button11rev.bmp bs=512 seek=1632

sudo dd of=$1 if=topleftcorner.bmp bs=512 seek=1648
sudo dd of=$1 if=bottomleftcorner.bmp bs=512 seek=1664
sudo dd of=$1 if=bottomrightcorner.bmp bs=512 seek=1680
sudo dd of=$1 if=topline.bmp bs=512 seek=1696
sudo dd of=$1 if=bottomline.bmp bs=512 seek=1712
sudo dd of=$1 if=sideline.bmp bs=512 seek=1728
sudo dd of=$1 if=maintitle.bmp bs=512 seek=1744
sudo dd of=$1 if=sysinfo.bmp bs=512 seek=1760
sudo dd of=$1 if=bootoptions.bmp bs=512 seek=1776

sudo dd of=$1 if=lb2.img bs=512 seek=1792
sudo dd of=$1 if=logo.bmp bs=512 seek=1808


