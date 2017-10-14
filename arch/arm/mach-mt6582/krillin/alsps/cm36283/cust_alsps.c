#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>
#include <mach/upmu_common.h>

static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 2,
	.polling_mode_ps =0,
	.polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    /* MTK: modified to support AAL */
    .als_level  = { 0, 45, 45, 45, 303, 303, 303, 1305, 1305, 2699, 2699, 7622, 38416, 46881, 65535},
    .als_value  = { 0, 80, 80, 80, 107, 107, 107,  650,  650, 1160, 1160, 3100,  5000,  8500, 10050, 10240},
    .ps_threshold_high = 31,
    .ps_threshold_low = 21,
};
struct alsps_hw *get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}

int pmic_ldo_suspend_enable(int enable)
{
	//0 for disable suspend, 1 for enable suspend
	upmu_set_vio18_lp_sel(enable);
	upmu_set_vio28_lp_sel(enable);
	return 0;
}



