include $(srctree)/drivers/misc/mediatek/Makefile.custom

# Module project related part


ifeq ($(CONFIG_MTK_SENSOR_SUPPORT),y)

# ALSPS (proximity sensor)
ifeq ($(CONFIG_MTK_CM36283),y)
obj-y	+=  alsps/cm36283/
endif
ifeq ($(CONFIG_MTK_TMD2772_AUTO),y)
obj-y	+=  alsps/tmd2772_auto/
endif

# Accelerometer
ifeq ($(CONFIG_MTK_LSM330),y)
obj-y   +=  accelerometer/lsm330/
endif

# Gyroscope
ifeq ($(CONFIG_MTK_L3GD20),y)
obj-y   +=  gyroscope/l3gd20/
endif

# Magnetometer
ifeq ($(CONFIG_MTK_ST480_AUTO),y)
obj-y	+=  magnetometer/st480_auto/
endif
ifeq ($(CONFIG_MTK_MMC3516X_AUTO),y)
obj-y	+=  magnetometer/mmc3516x_auto/
endif

endif

ifneq ($(CONFIG_ARCH_MTK_PROJECT),)
ARCH_MTK_PROJECT := $(shell echo $(CONFIG_ARCH_MTK_PROJECT))
obj-y	+= core/
obj-y	+= dct/dct/
endif

obj-y   +=  lcm/
obj-y	+=  camera/camera/
obj-y	+=  vibrator/
obj-y	+=  leds/
obj-y	+=  keypad/
obj-y	+=  sound/
obj-y	+=  accdet/
obj-y	+=  partition/
