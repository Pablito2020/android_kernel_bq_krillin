include $(srctree)/drivers/misc/mediatek/Makefile.custom

# In case the platform does NOT support this type of sensors
obj-y   +=  mag.o
obj-y   +=  mag_factory.o

ifeq ($(CONFIG_MTK_ST480_AUTO),y)
obj-y	+=  st480_auto/
endif

ifeq ($(CONFIG_MTK_MMC3516X_AUTO),y)
obj-y	+=  mmc3516x_auto/
endif
