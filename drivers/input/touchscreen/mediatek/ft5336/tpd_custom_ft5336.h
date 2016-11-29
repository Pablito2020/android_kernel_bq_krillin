#ifndef TOUCHPANEL_H__
#define TOUCHPANEL_H__

/* Pre-defined definition */
#define TPD_TYPE_CAPACITIVE
#define TPD_TYPE_RESISTIVE
#define TPD_I2C_NUMBER           0
#define TPD_WAKEUP_TRIAL         60
#define TPD_WAKEUP_DELAY         100

#define VELOCITY_CUSTOM
#define TPD_VELOCITY_CUSTOM_X 15
#define TPD_VELOCITY_CUSTOM_Y 15

#define TPD_POWER_SOURCE_CUSTOM         MT6323_POWER_LDO_VGP1

#define TPD_DELAY                (2*HZ/100)
#define TPD_CALIBRATION_MATRIX  {962,0,0,0,1600,0,0,0};
#define TPD_HAVE_BUTTON
#define TPD_BUTTON_HEIGH        (100)
#define TPD_KEY_COUNT           3
#define TPD_KEYS                {KEY_MENU, KEY_HOMEPAGE, KEY_BACK}
#define TPD_KEYS_DIM            {{80,1020,100,100},{240,1020,100,100},{400,1020,100,100}}


//#define FTS_GESTRUE
#define FTS_PRESSURE
#define LCM_NAME1 "hx8389_qhd_dsi_vdo_truly"
#define LCM_NAME2 "hx8389b_qhd_dsi_vdo_tianma"

static unsigned char CTPM_FW[]=
{
#include "FT5336_HiKe_Krilin_OGS_540X960_Truly0x5a_Ver0x15_20140618_app.i"
};

static unsigned char CTPM_FW2[]=
{
#include "FT5336_Hike_Krilin_540X960_LaiBao0x55_Ver0x12_20140729_app.i"
};
#endif /* TOUCHPANEL_H__ */
