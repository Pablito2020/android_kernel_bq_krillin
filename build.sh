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
echo -e "${orange}####################################################"
echo -e "${orange}#                  SCRIPT INFO:                    #"
echo -e "${orange}#             DEVELOPER: PABLITO2020               #"
echo -e "${orange}#       THANKS TO: ASSUSDAN, AND GUYS OF 4PDA      #"
echo -e "${orange}####################################################"

# EXPORT GOOGLE GCC TOOLCHAIN
export ARCH=arm CROSS_COMPILE=../arm-eabi-4.8/bin/arm-eabi-

# User and Build Host
export KBUILD_BUILD_USER=Pablito2020
export KBUILD_BUILD_HOST=Daredevil

# READ THE Lineage/AOSP DEFCONFIG
echo -e "${cyan} CONFIGURE KRILLIN.."
make lineage_krillin_defconfig

# BUILD ZIMAGE ( I ADD -J4 BECAUSE IS MY PC CORES.... CHANGE THIS NUMBER FOR YOUR PC NUMBER OF CORES )
# For check the pc cores open a terminal and write: grep -c ^processor /proc/cpuinfo
echo -e "${purple} BUILD DAREDEVIL KERNEL FOR KRILLIN.."
make -j4 zImage

# COMPROBE IF THERE ARE COMPILATION ERRORS
if [ ! -f arch/arm/boot/zImage ]
then
    echo -e "${red}############################"
    echo -e "${red}#        BUILD ERROR!      #"
    echo -e "${red}############################"
else

# IF THE KERNEL COMPILES SUCCESFULLY
echo -e "${green} ########################################################"
echo -e "${green} #          SUCCESSFULLY BUILDED DAREDEVIL KERNEL       #"
echo -e "${green} #           ZIMAGE IS IN ARCH/ARM/BOOT/ZIMAGE          #"
echo -e "${green} #         MAKE BOOT.IMG WITH CARLIV IMAGE KITCHEN      #"
echo -e "${green} #           HAVE FUN AND GIVE PROPER CREDITS!          #"
echo -e "${green} ########################################################"
fi

# BUILD WORKTIME
echo $[$SECONDS / 60]' minutes '$[$SECONDS % 60]' seconds' 

