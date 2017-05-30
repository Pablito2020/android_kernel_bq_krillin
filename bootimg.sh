#!/bin/bash
# SCRIPT FOR BUILD BOOTIMG 
# BASED ON HYPERION70 SCRIPT
# MOFIFIED BY PABLITO2020 FOR THE 3.10 LINUX KERNEL SOURCE CODE
PRJ=krillin
 echo " Generating boot.img  ..."
 boot-img/acp -uv arch/arm/boot/kernel_${PRJ}.bin arch/arm/boot/kernel
 boot-img/mkbootfs boot-img/host/root | boot-img/minigzip > arch/arm/boot/ramdisk.img
 boot-img/mkimage arch/arm/boot/ramdisk.img ROOTFS > arch/arm/boot/ramdisk_android.img
 mv arch/arm/boot/ramdisk.img boot-img/mediatek/${PRJ}_ramdisk.img
 # mv arch/arm/boot/ramdisk_android.img out/target/product/${PRJ}/ramdisk.img // Don't know what to do with this for now...
 boot-img/mkbootimg  --kernel arch/arm/boot/kernel --ramdisk arch/arm/boot/ramdisk.img --board 1336460062 --output arch/arm/boot/boot.img
 echo "####################################"
  echo "#   BOOT.IMG SUCCESFULLY BUILDED! #"
  echo "###################################"
