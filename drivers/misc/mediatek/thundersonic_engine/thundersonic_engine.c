/*
 * Copyright © 2015, Varun Chitre "varun.chitre15" <varun.chitre15@gmail.com>
 *
 * Sound Control Driver for MTK Sound SoC
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Please preserve this licence and driver name if you implement this
 * anywhere else.
 *
 */

#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <mach/upmu_hw.h>
#include "../sound/mt6582/AudDrv_Common.h"
#include "thundersonic_defs.h"

#define THUNDERSONIC "thundersonic"

#define ENGINE_VERSION  3
#define ENGINE_VERSION_SUB 0

extern void Ana_Set_Reg(uint32 offset, uint32 value, uint32 mask);
extern uint32 Ana_Get_Reg(uint32 offset);

int cust_hpl_index = 4;
int cust_hpr_index = 4;
int cust_hs_index = 11;
bool lockhs = false;
bool lockhp = false;

static void setHPLGain(void) {
	uint32 mask, index;
	mask = 0x00007000 | 0xffff0000;
	index = (uint32) cust_hpl_index;
	lockhp = false;
	Ana_Set_Reg(AUDTOP_CON5, index, mask);
	lockhp = true;
}

static void setHPRGain(void) {
	uint32 mask, index;
	mask = 0x000000700 | 0xffff0000;
	index = (uint32) cust_hpr_index;
	lockhp = false;
	Ana_Set_Reg(AUDTOP_CON5, index, mask);
	lockhp = true;
}

static void setHSGain(void) {
	uint32 mask, index;
	mask = 0x000000f0 | 0xffff0000;
	index = (uint32) cust_hs_index;
	lockhs = false;
	Ana_Set_Reg(AUDTOP_CON7, index, mask);
	lockhs = true;
}

static ssize_t hplgain_reg_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	uint32 currentVol;
	int val;
	currentVol = Ana_Get_Reg(AUDTOP_CON5) >> 12;
	currentVol &= 0x7;
	val = (int) currentVol;
	return sprintf(buf, "%d\n", val);
}

static ssize_t hprgain_reg_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	uint32 currentVol;
	int val;
	currentVol = Ana_Get_Reg(AUDTOP_CON5) >> 8;
	currentVol &= 0x7;
	val = (int) currentVol;
	return sprintf(buf, "%d\n", val);
}

static ssize_t hplgain_reg_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned int val;
	sscanf(buf, "%u", &val);
		cust_hpl_index = (uint32) val;
		setHPLGain();

	return count;
}

static ssize_t hprgain_reg_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned int val;
	sscanf(buf, "%u", &val);
		cust_hpr_index = (uint32) val;
		setHPRGain();

	return count;
}

static ssize_t hsgain_reg_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	uint32 currentVol;
	int val;
	currentVol = Ana_Get_Reg(AUDTOP_CON7) >> 4;
	currentVol &= 0xf;
	val = (int) currentVol;
	return sprintf(buf, "%d\n", val);
}

static ssize_t hsgain_reg_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned int val;
	sscanf(buf, "%u", &val);
		cust_hs_index = (uint32) val;
		setHSGain();

	return count;
}

static ssize_t thundersonic_version_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "version: %u.%u\n", ENGINE_VERSION, ENGINE_VERSION_SUB);
}

static struct kobj_attribute hplgain_attribute =
	__ATTR(hpl_gain,
		0666,
		hplgain_reg_show, hplgain_reg_store);

static struct kobj_attribute hprgain_attribute =
	__ATTR(hpr_gain,
		0666,
		hprgain_reg_show, hprgain_reg_store);

static struct kobj_attribute hsgain_attribute =
	__ATTR(hs_gain,
		0666,
		hsgain_reg_show, hsgain_reg_store);

static struct kobj_attribute thundersonic_version_attribute =
	__ATTR(engine_version,
		0444,
		thundersonic_version_show, NULL);

static struct attribute *thundersonic_engine_attrs[] =
	{
		&hplgain_attribute.attr,
		&hprgain_attribute.attr,
		&hsgain_attribute.attr,
		&thundersonic_version_attribute.attr,
		NULL,
	};

static struct attribute_group hp_gain_level_control_attr_group =
	{
		.attrs = thundersonic_engine_attrs,
	};

static struct kobject *hp_gain_control_kobj;

static int hp_gain_control_init(void)
{
	int sysfs_result;
	printk(KERN_DEBUG "[%s]\n",__func__);

	hp_gain_control_kobj =
		kobject_create_and_add("thundersonic_engine", kernel_kobj);

	if (!hp_gain_control_kobj) {
		pr_err("%s Interface create failed!\n",
			__FUNCTION__);
		return -ENOMEM;
        }

	sysfs_result = sysfs_create_group(hp_gain_control_kobj,
			&hp_gain_level_control_attr_group);

	if (sysfs_result) {
		pr_info("%s sysfs create failed!\n", __FUNCTION__);
		kobject_put(hp_gain_control_kobj);
	}
	return sysfs_result;
}

static void hp_gain_control_exit(void)
{
	if (hp_gain_control_kobj != NULL)
		kobject_put(hp_gain_control_kobj);
}

module_init(hp_gain_control_init);
module_exit(hp_gain_control_exit);
MODULE_LICENSE("GPL and additional rights");
MODULE_AUTHOR("Varun Chitre <varun.chitre15@gmail.com>");
MODULE_DESCRIPTION("ThunderSonic Engine - Sound Control Module for MTK SoCs");

