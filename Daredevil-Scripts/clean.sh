#!/bin/bash
#
# Daredevil Kernel clean script
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

# Variables
e=echo
m=make

# This is needed since we have the script on the Daredevil-Script folder
cd ..

# Say info
$e '################################# '
$e '#                               # '
$e '#  CLEANING THE KERNEL SOURCE   # '
$e '#                               # '
$e '################################# '

$m clean
$m mrproper
if [ ! -f build-log.txt ]
then
    $e -e "############################"
    $e -e "#   NO BUILD LOG FOUND     #"
    $e -e "############################"
else
rm -rf build-log.txt
fi

# Now clear the terminal log.
clear
