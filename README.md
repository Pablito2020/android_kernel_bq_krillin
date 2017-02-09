# ALPS Kernel for krillin

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

* Bugs (not working):

  * APP_SWITCH button (button number 252) 
    

# Compilation guide:
  
  * First, made a folder for cm kernel and enter to it:

        $ mkdir CM_KERNEL

        $ cd CM_KERNEL

  * Then, clone the project: 

        $ https://github.com/Pablito2020/cyanogenmod_kernel_bq_krillin.git

  * After that, clone the toolchain used in this build (in that case gcc 4.8): 

        $ git clone https://android.googlesource.com/platform/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8

  * And for compile do that:

        $ cd cyanogenmod_kernel_bq_krillin

        $ . build.sh


# Thanks to:
   * @zormax for the cm12/13 project
   * @assusdan for help me a lot with this kernel
   * BQ for his sources
    
