
include $(srctree)/drivers/misc/mediatek/Makefile.custom

# Module project related part

ifeq ($(CONFIG_MTK_SENSOR_SUPPORT),y)
ifeq ($(CONFIG_MTK_STK3X1X),y)
obj-y	+=  alsps/stk3x1x/
endif

ifeq ($(CONFIG_MTK_TMD2772_AUTO),y)
obj-y	+=  alsps/tmd2772/
endif

ifeq ($(CONFIG_MTK_KXTJ2_1009),y)
obj-y   +=  accelerometer/KXTJ2_1009/
endif


ifeq ($(CONFIG_MTK_LSM303D),y)
obj-y   +=  accelerometer/lsm303d/
endif

endif

ifneq ($(CONFIG_ARCH_MTK_PROJECT),)
ARCH_MTK_PROJECT := $(shell echo $(CONFIG_ARCH_MTK_PROJECT))
obj-y	+= core/
obj-y	+= dct/dct/
endif

obj-y   +=  lcm/
#obj-y	+=  hdmi/
obj-y	+=  camera/camera/
obj-y	+=  imgsensor/ov5648_mipi_raw/
obj-y	+=  imgsensor/sp2529_mipi_yuv/
obj-y   +=  flashlight/constant_flashlight/
obj-y	+=  vibrator/
obj-y	+=  leds/
obj-y	+=  keypad/
obj-y	+=  sound/
obj-y	+=  accdet/
obj-y	+=  partition/

