export ARCH=arm CROSS_COMPILE=../android_toolchain/linaro-4.9/bin/arm-linux-androideabi-
export KBUILD_BUILD_USER=assusdan
export KBUILD_BUILD_HOST=SRT

    #For checking errors
echo 'Remove kernel...'
rm -rf arch/arm/boot/zImage


echo 'Configure CM Zera S '
make alps_defconfig >/dev/null

echo 'Building CM Zera S'
make -j4 zImage >/dev/null 2>buildlog.log

    #check errors
if [ ! -f arch/arm/boot/zImage ]
then
    echo "BUID ERRORS!"
else
 #if OK
echo 'Moving CM Zera S'
mv arch/arm/boot/zImage zImage # /var/www/compiled/CM-zImage-zeras
fi


#write worktime
echo $[$SECONDS / 60]' minutes '$[$SECONDS % 60]' seconds' 
