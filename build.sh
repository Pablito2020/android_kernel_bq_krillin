#!/bin/bash

#Set Color
blue='\033[0;34m'
cyan='\033[0;36m'
yellow='\033[0;33m'
red='\033[0;31m'
nocol='\033[0m'

export ARCH=arm CROSS_COMPILE=../arm-eabi-4.8/bin/arm-eabi-
export KBUILD_BUILD_USER=pablito
export KBUILD_BUILD_HOST=htcmania

#For checking errors
echo "$cyan Remove kernel..."
rm -rf arch/arm/boot/zImage

echo "$cyan Configure krillin.... "
make alps_defconfig

echo "$cyan Building CM for krillin...."
make zImage


echo Kernel succesfully builded
echo "#Pablito2020"
