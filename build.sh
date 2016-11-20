export ARCH=arm CROSS_COMPILE=../toolchain/ubertc-5.3/bin/arm-eabi-
export KBUILD_BUILD_USER=pablito
export KBUILD_BUILD_HOST=htcmania

    #For checking errors
echo 'Remove kernel...'
rm -rf arch/arm/boot/zImage


echo 'Configure krillin.... '
make alps_defconfig >/dev/null

echo 'Building CM for krillin....'
make -j4 zImage >/dev/null 2>buildlog.log

    #check errors
if [ ! -f arch/arm/boot/zImage ]
then
    echo "BUID ERRORS!"
else
 #if OK
echo 'Moving CM krillin zimage'
mv arch/arm/boot/zImage /var/www/compiled/CM-zImage-zeras
fi


#write worktime
echo $[$SECONDS / 60]' minutes '$[$SECONDS % 60]' seconds' 
