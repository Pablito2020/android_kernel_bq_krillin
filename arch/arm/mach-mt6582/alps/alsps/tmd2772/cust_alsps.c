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

   .als_level  = { 10,  32,  50,  70,  90,  128,  180,   640,  896,   1284,   1652,  2164,   2676,   3410,  4200,  5224},
  .als_value  = { 0, 20, 160, 640, 640, 2500, 5000,  5000,  5000,  5000,  6000, 6000,  6000, 9000,  9999, 10240},
    .ps_threshold_high = 500,
    .ps_threshold_low = 400,
    .ps_threshold = 500,
};
struct alsps_hw *get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}

int TMD2772_CMM_PPCOUNT_VALUE = 0x10;
int TMD2772_ZOOM_TIME = 60;
int TMD2772_CMM_CONTROL_VALUE = 0x20;
//add by sen.luo 0xA0=25MA,0x20=100MA,0xE0=12.5MA,0x60=50ma

