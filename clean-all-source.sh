#!/bin/bash
# BASH FILE FOR CLEAN ALL THE SOURCE WITHOUT COMPILE ALL THE KERNEL AGAIN AND AGAIN
# PABLITO2020 2017

rm -rf pablito-scripts/zip/boot.img
rm -rf arch/arm/boot/kernel
rm -rf arch/arm/boot/ramdisk.img
rm -rf arch/arm/boot/ramdisk_android.img
make clean
make mrproper
rm -rf pablito-scripts/zip/DAREDEVIL-KERNEL-KRILLIN.zip


