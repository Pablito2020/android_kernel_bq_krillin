# 3.10.107 CUSTOM KERNEL FOR KRILLIN

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
  * Sweep2Wake + Doubletap2Wake

# Compilation guide:
  
  * First, made a folder for the kernel and enter to it:

        $ mkdir kernel

        $ cd kernel

  * Then, clone the project: 

        $ https://github.com/Pablito2020/android_kernel_bq_krillin.git

  * After that, clone the toolchain used in this build (in that case linaro 4.9): 

        $ git clone https://github.com/Pablito2020/Toolchain.git

  * And for compile:

        $ mv android_kernel_bq_krillin kernel

        $ cd kernel

        $ . build.sh

  * The kernel file (called zImage, will be found in arch/arm/boot folder)

# If you want to clean all the source without compiling execute this comand on the principal kernel diretory:

        $ . clean.sh

# Developer:

   * Pablito2020

# Thanks to (this people are persons who help me to adapt the kernel source to my device "krillin") There's why I put them here, because they don't have any specific commit,so.. They are here! :) :

   * ANDR7E ( help with kpd buttons + device info app for mtk devices) ---- XDA, 4PDA, GITHUB
   * @assusdan help me with drivers and develop the kernel. ---- 4PDA, GITHUB
   * BQ for his sources (LCM DRVIERS, TOUCH DRIVERS, ETC) ---- GITHUB
   * Vineeth Raj for his dt2wake and sweep2wake commits for sprout. ---- GITHUB
   * Varun Chitre for his "thunder" drivers for sprout. ---- GITHUB
   * @zormax for the bring up LP/MM project ---- 4PDA

# Filesystems that the kernel supports:
   *EXT4
   *EXT3
   *EXT2
   *NTFS
   *F2FS
   *FUSE
   *SDCARDFS

# UPDATED DRIVERS (from the google source common kernel):

  * Android Binder
  * LZ4 Compression Driver 
  * SDCardFS
  * EXT4
