include $(srctree)/drivers/misc/mediatek/Makefile.custom

# In case the platform does NOT support this type of sensors
obj-y   +=  gyroscope.o gyro_factory.o

ifeq ($(CONFIG_MTK_L3GD20),y)
obj-y   +=  l3gd20/
endif
