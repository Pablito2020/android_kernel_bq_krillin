#!/bin/bash

# SCRIPT FOR BUILD BOOTIMG 
# BASED ON HYPERION70 SCRIPT
# MOFIFIED BY PABLITO2020 FOR THE 3.10 LINUX KERNEL SOURCE CODE FOR KRILLIN

 echo " Generating boot.img  ..."
 cd ..
 mv arch/arm/boot/zImage arch/arm/boot/kernel
 pablito-scripts/acp -uv arch/arm/boot/kernel
 pablito-scripts/mkbootfs pablito-scripts/host/root | pablito-scripts/minigzip > arch/arm/boot/ramdisk.img
 pablito-scripts/mkimage arch/arm/boot/ramdisk.img ROOTFS > arch/arm/boot/ramdisk_android.img
 pablito-scripts/mkbootimg  --kernel arch/arm/boot/kernel --ramdisk arch/arm/boot/ramdisk.img --board 1336460062 --output arch/arm/boot/boot.img
 echo "####################################"
 echo "#   BOOT.IMG SUCCESFULLY BUILDED! #"
 echo "###################################"
