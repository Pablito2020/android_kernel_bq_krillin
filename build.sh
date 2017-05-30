#!/bin/bash


# SET COLORS! (SOME OF THEM ARE NOT USED, BUT IF YOU LIKE, YOU CAN ADD IT IN THE ECHO LINES)
blue='\033[0;34m'
cyan='\033[0;36m'
yellow='\033[0;33m'
green='\033[0;32m'
red='\033[0;31m'
nocol='\033[0m'
orange='\033[0;33m'
light_red='\033[1;31m'
purple='\033[0;35m'


# SAY INFO ABOUT THE KERNEL AND THE SCRIPT
echo -e "${orange}####################################################"
echo -e "${orange}#      INFO ABOUT THE KERNEL AND THE SCRIPT        #"
echo -e "${orange}####################################################"
echo -e "${orange}#                  KERNEL INFO:                    #"
echo -e "${orange}#             DEVICE: BQ AQUARIS E4.5              #"
echo -e "${orange}#          LINUX KERNEL VERSION: 3.10.49           #"
echo -e "${orange}#                VERSION: STABLE                   #"
echo -e "${orange}####################################################"
echo -e "${orange}#                  SCRIPT INFO:                    #"
echo -e "${orange}#             DEVELOPER: PABLITO2020               #"
echo -e "${orange}#           CONTRIBUTORS: HYPERION70               #"
echo -e "${orange}####################################################"


# EXPORT TOOLCHAIN + SAY USER AND HOST BUILDER
export ARCH=arm CROSS_COMPILE=../arm-eabi-4.8/bin/arm-eabi-
export KBUILD_BUILD_USER=pablito
export KBUILD_BUILD_HOST=developer


# IF YOU HAVE BUILDED THE KERNEL, AND YOU WANT TO RECOMPILE IT, CLEAN THE SOURCE
if test -e arch/arm/boot/kernel; then
echo -e "${cyan} CLEAN THE SOURCE..."
rm -rf arch/arm/boot/kernel
rm -rf arch/arm/boot/ramdisk.img
rm -rf arch/arm/boot/ramdisk_android.img
make clean
make mrproper
fi


# IF A OLD BOOT.IMG FILE EXISTS, DELETE IT.
if test -e pablito-scripts/zip/boot.img; then
echo -e "${cyan} DELETE OLD BOOT.IMG FILE..."
rm -rf pablito-scripts/zip/boot.img
fi

# IF A OLD DAREDEVIL.ZIP FILE EXISTS, DELETE IT.
if test -e pablito-scripts/DAREDEVIL-KERNEL-KRILLIN.zip; then
echo -e "${cyan} DELETE OLD FLASHEABLE .ZIP KERNEL FILE..."
rm -rf pablito-scripts/DAREDEVIL-KERNEL-KRILLIN.zip
fi


# READ THE DEFAULT DEFCONFIG
echo -e "${cyan} CONFIGURE KRILLIN.."
make lineage_krillin_defconfig


# BUILD ZIMAGE
echo -e "${purple} BUILD DAREDEVIL KERNEL FOR KRILLIN.."
make zImage 


# GENERATE BOOT.IMG
echo -e "${purple} BUILD BOOT.IMG FILE.."
cd pablito-scripts
. bootimg.sh


# CREATE .ZIP FILE
echo -e "${purple} CREATE A FLASHEABLE .ZIP FILE"
mv arch/arm/boot/boot.img pablito-scripts/zip/boot.img
cd pablito-scripts/zip
zip -r -0 DAREDEVIL-KERNEL-KRILLIN.zip ./*

# CHANGE DIRECTORY
cd ..
cd ..

# SAY YES TO A SUCCESFULLY BUILD :)
echo -e "${green} SUCCESSFULLY BUILDED DAREDEVIL KERNEL"
echo -e "${green} FLASHEABLE KERNEL.ZIP IN pablito-scripts/zip folder"
echo -e "${green} HAVE FUN AND GIVE PROPER CREDITS!"
echo -e $[$SECONDS / 60]' minutes '$[$SECONDS % 60]' seconds'

