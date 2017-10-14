#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>
#include <mach/upmu_common.h>

static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 0,
	.polling_mode_ps =0,
	.polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    /* MTK: modified to support AAL */
	 .als_level  = {0,3, 9, 22,  40, 70, 140,280,   560,650,1900,5800, 10240,10241},
     .als_value  = {0,18,60,100, 220,230,250,300,   500,560,1200,2600, 10240,10240},
    .ps_threshold_high = 500,
    .ps_threshold_low = 400,
  
};
struct alsps_hw *TMD2772_get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}

int TMD2772_CMM_PPCOUNT_VALUE = 0x0d;
int TMD2772_ZOOM_TIME = 60;
int TMD2772_CMM_CONTROL_VALUE = 0x22;
