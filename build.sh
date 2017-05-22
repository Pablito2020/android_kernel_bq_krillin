#!/bin/bash

# Set colors!
blue='\033[0;34m'
cyan='\033[0;36m'
yellow='\033[0;33m'
green='\033[0;32m'
red='\033[0;31m'
nocol='\033[0m'
orange='\033[0;33m'
light_red='\033[1;31m'
purple='\033[0;35m'


# Export toolchain + build user and host
export ARCH=arm CROSS_COMPILE=../arm-eabi-4.8/bin/arm-eabi-
export KBUILD_BUILD_USER=pablito
export KBUILD_BUILD_HOST=developer

#For checking errors
echo -e "${cyan} Remove kernel..."
rm -rf arch/arm/boot/zImage

echo -e "${cyan} Configure krillin.... "
make lineage_krillin_defconfig

echo -e "${purple} Building DareDevil 6.0 for krillin...."
make zImage

echo -e "${green} Succesfully builded DareDevil Kernel!"
echo -e $[$SECONDS / 60]' minutes '$[$SECONDS % 60]' seconds'

