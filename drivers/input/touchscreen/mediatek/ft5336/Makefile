include $(srctree)/drivers/misc/mediatek/Makefile.custom

# Linux driver folder
ccflags-y += -I$(srctree)/arch/arm/mach-$(MTK_PLATFORM)/$(ARCH_MTK_PROJECT)/touchpanel/ft5336/
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/ft5336/
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/
# Compile ft5336
obj-y	+=  ft5336_driver.o
