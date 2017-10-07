#!/bin/bash
#
# Daredevil Kernel compilation script
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

# Say info about the script in terminal
echo -e "${orange}####################################################"
echo -e "${orange}#                                                  #"
echo -e "${orange}#                  SCRIPT INFO:                    #"
echo -e "${orange}#  THIS IS A SCRIPT FOR BUILDING DAREDEVIL KERNEL  #"
echo -e "${orange}#     NOW, PLEASE SELECT THE TOOLCHAIN YOU WANT    #"
echo -e "${orange}#            FOR THE KERNEL COMPILATION            #"
echo -e "${orange}# OR SELECT THE CLEAN THE KERNEL SOURCE OPTION FOR #"
echo -e "${orange}#           A MAKE CLEAN AND MAKE PROPER           #"
echo -e "${orange}#                                                  #"
echo -e "${orange}####################################################"

# Enter to the Daredevil-Scripts folder
cd Daredevil-Scripts

# Say what toolchain do you want to build
echo
  echo "Select what do you want to do:"
  echo
  echo "  1 - Build Daredevil with the Google gcc 4.8 toolchain"
  echo "  2 - Build Daredevil with the Linaro 7.2.1 toolchain"
  echo "  3 - Clean the kernel source"
  echo "  x - Exit"
  echo
  echo -n "Enter Option: "
read opt

	case $opt in
		1) . build-gcc.sh;;
		2) . build-linaro.sh;;
		3) . clean.sh;;
		x) clear; echo; echo "Goodbye."; echo; exit 1;;
		*) ERR_MSG="Invalid option!"; clear;;
esac
