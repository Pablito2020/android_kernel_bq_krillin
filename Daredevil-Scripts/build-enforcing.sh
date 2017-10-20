#!/bin/bash
#
# Daredevil Kernel compiler script for gcc 4.8 toolchain
#
# Copyright (C) 2017 Pablo Fraile Alonso (Github aka: Pablito2020)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>


# Since we have the script in the Daredevil-Scripts folder that is needed:
cd ..

# Set Variables!
blue='\033[0;34m'
cyan='\033[0;36m'
yellow='\033[0;33m'
green='\033[0;32m'
red='\033[0;31m'
nocol='\033[0m'
orange='\033[0;33m'
light_red='\033[1;31m'
purple='\033[0;35m'
m=make
echo=e

# If the google toolchain 4.8 doesn't exist, clone it. 
# If exists, export the toolchain path
if [ ! -f ../arm-eabi-4.8/bin/arm-eabi-addr2line ]
then
    $e -e "####################################"
    $e -e "#       TOOLCHAIN NOT FOUND!       #"
    $e -e "####################################"
cd ..
git clone https://android.googlesource.com/platform/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8
export ARCH=arm CROSS_COMPILE=../arm-eabi-4.8/bin/arm-eabi-
else
export ARCH=arm CROSS_COMPILE=../arm-eabi-4.8/bin/arm-eabi-
fi


# User and Build Host
export KBUILD_BUILD_USER=pablito
export KBUILD_BUILD_HOST=linuxmachine


# Read the lineage/AOSP DEFCONFIG
$e -e "${orange} Read the defconfig file ..."
$m lineage_krillin_defconfig


# Build zImage (Thanks to Joel for the all command)
$e -e "${orange} Building Daredevil Kernel for krillin ..."
$m -j4 zImage


# Check if there are errors in the kernel
if [ ! -f arch/arm/boot/zImage ]
then
    $e -e "${red}############################"
    $e -e "${red}#        BUILD ERROR!      #"
    $e -e "${red}############################"
else

# If the kernel compiles succesfully
$e -e "${green} #########################################"
$e -e "${green} #                                       #"
$e -e "${green} # SUCCESSFULLY BUILDED DAREDEVIL KERNEL #"
$e -e "${green} #        $[$SECONDS / 60]' minutes '$[$SECONDS % 60]' seconds'       #" 
$e -e "${green} #                                       #"
$e -e "${green} #########################################"
fi
