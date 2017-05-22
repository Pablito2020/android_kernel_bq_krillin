include $(srctree)/drivers/misc/mediatek/Makefile.custom

# In case the platform does NOT support this type of sensors
obj-y   +=  alsps.o
obj-y   +=  alsps_factory.o
obj-y   +=  aal_control.o

ifeq ($(CONFIG_MTK_CM36283),y)
obj-y	+=  cm36283/
endif
	
ifeq ($(CONFIG_MTK_TMD2772_AUTO),y)
obj-y	+=  tmd2772_auto/
endif
