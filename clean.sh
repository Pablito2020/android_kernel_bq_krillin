#!/bin/bash
# Script for clean the DareDevil kernel source
# Pablo Fraile Alonso (Pablito2020), 2017

echo '################################# '
echo '#                               # '
echo '#  CLEANING THE KERNEL SOURCE   # '
echo '#                               # '
echo '################################# '

make clean
make mrproper
if [ ! -f build-log.txt ]
then
    echo -e "############################"
    echo -e "#   NO BUILD LOG FOUND     #"
    echo -e "############################"
else
rm -rf build-log.txt
fi
# Now clear the terminal log.
clear


