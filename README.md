# LINEAGE OS KERNEL FOR BQ AQUARIS E4.5, AKA KRILLIN

* Working:

  * LCM Drivers (hx8389b_qhd_dsi_vdo_tianma, hx8389_qhd_dsi_vdo_truly)
  * Touchscreen Driver (FT5336)
  * Batery Table
  * Vibrator driver
  * Rear Camera
  * Flashlight
  * Sound driver (thanks to vo-1)
  * Frontal Camera
  * Storage (emulated storage and sdcard working)
  * Acceleremoter
  * Light sensor
  * Proximity sensor
  * WIFI
  * Bluetooth
  * GPS
  * VPN
  * Gyroscope sensor

* Bugs (NOT WORKING):

  * You tell me.... Everything seems to work

# Compilation guide:
  
  * First, made a folder for the kernel and enter to it:

        $ mkdir kernel

        $ cd kernel

  * Then, clone the project: 

        $ https://github.com/Pablito2020/android_kernel_bq_krillin.git

  * After that, clone the toolchain used in this build (in that case gcc 4.8): 

        $ git clone https://android.googlesource.com/platform/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8

  * And for compile:

        $ mv android_kernel_bq_krillin kernel

        $ cd kernel

        $ . build.sh

  * Flasheable .zip file will be in the principal kernel directory with the name DAREDEVIL-KERNEL-KRILLIN.zip

# If you want to clean all the source without compiling execute this comand on the principal kernel diretory:

        $ . clean-all-source.sh

# Developer:

   * Pablito2020

# Thanks to (In alphabetical order) :

   * ANDR7E ( help with kpd buttons + device info app for mtk devices) ---- XDA, 4PDA, GITHUB
   * @assusdan help me with drivers and develop the kernel. ---- 4PDA, GITHUB
   * BQ for his sources (LCM DRVIERS, TOUCH DRIVERS, ETC) ---- GITHUB
   * @zormax for the bring up LP/MM project ---- 4PDA
    
