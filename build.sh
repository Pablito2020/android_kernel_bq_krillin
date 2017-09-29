#!/bin/bash

# Set Colors! (some of there aren't used, but if you like, you can add it in the echo lines.)
blue='\033[0;34m'
cyan='\033[0;36m'
yellow='\033[0;33m'
green='\033[0;32m'
red='\033[0;31m'
nocol='\033[0m'
orange='\033[0;33m'
light_red='\033[1;31m'
purple='\033[0;35m'

# Say info about the kernel and the script in terminal
echo -e "${orange}####################################################"
echo -e "${orange}#      INFO ABOUT THE KERNEL AND THE SCRIPT        #"
echo -e "${orange}####################################################"
echo -e "${orange}#                                                  #"
echo -e "${orange}#                  KERNEL INFO:                    #"
echo -e "${orange}#             DEVICE: BQ AQUARIS E4.5              #"
echo -e "${orange}#               DEVICE AKA: krillin                #"
echo -e "${orange}#          LINUX KERNEL VERSION: 3.10.107          #"
echo -e "${orange}#                                                  #"
echo -e "${orange}####################################################"
echo -e "${orange}#                                                  #"
echo -e "${orange}#                  SCRIPT INFO:                    #"
echo -e "${orange}#             DEVELOPER: PABLITO2020               #"
echo -e "${orange}#       THANKS TO: ASSUSDAN, AND GUYS OF 4PDA      #"
echo -e "${orange}#                                                  #"
echo -e "${orange}####################################################"

# Export google 4.8 toolchain
export ARCH=arm CROSS_COMPILE=../arm-eabi-4.8/bin/arm-eabi-

# User and Build Host
export KBUILD_BUILD_USER=pablito
export KBUILD_BUILD_HOST=linuxmachine

# Read the lineage/AOSP DEFCONFIG
echo -e "${cyan} CONFIGURE KRILLIN.."
make lineage_krillin_defconfig

# Build zImage (Thanks to Joel for the all command)
echo -e "${purple} BUILD DAREDEVIL KERNEL FOR KRILLIN.."
make -j all zImage > build-log.txt

# Check if there are errors in the kernel
if [ ! -f arch/arm/boot/zImage ]
then
    echo -e "${red}############################"
    echo -e "${red}#        BUILD ERROR!      #"
    echo -e "${red}############################"
else

# If the kernel compiles succesfully
echo -e "${green} ########################################################"
echo -e "${green} #                                                      #"
echo -e "${green} #          SUCCESSFULLY BUILDED DAREDEVIL KERNEL       #"
                  echo $[$SECONDS / 60]' minutes '$[$SECONDS % 60]' seconds' 
echo -e "${green} #                                                      #"
echo -e "${green} ########################################################"
fi

# Build worktime ( added in line 59 for now.)
# echo $[$SECONDS / 60]' minutes '$[$SECONDS % 60]' seconds' 

