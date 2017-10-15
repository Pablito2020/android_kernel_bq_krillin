include $(srctree)/drivers/misc/mediatek/Makefile.custom

# In case the platform does NOT support this type of sensors
obj-y   +=  accel.o accel_factory.o

# Accelerometer device related drivers
ifeq ($(CONFIG_MTK_LSM330),y)
obj-y   +=  lsm330/
endif

