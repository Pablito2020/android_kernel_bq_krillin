#ifdef BUILD_LK
#else
#include <linux/string.h>
#ifndef BUILD_UBOOT
#include <linux/kernel.h>
#endif
#endif
#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (540)
#define FRAME_HEIGHT (960)


#define LCM_ID_OTM9605A					0x9605

#ifndef TRUE
    #define   TRUE     1
#endif

#ifndef FALSE
    #define   FALSE    0
#endif
static unsigned int lcm_esd_test = 0;      ///only for ESD test
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))
#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0x1FF   // END OF REGISTERS MARKER

extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int *rawdata);

extern void CKT_SET_HS_READ(void);
extern void CKT_RESTORE_HS_READ(void);
// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)									lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)				lcm_util.dsi_write_regs(addr, pdata, byte_nums)
//#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

       

#define LCM_DSI_CMD_MODE 0

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {
	
	/*
	Note :

	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05

	Structure Format :

	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},

	...

	Setting ending by predefined flag
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/

	{0x00,	1,	{0x00}},
	{0xff,	3,	{0x96,0x05,0x01}},
	{0x00,	1,	{0x80}},
	{0xff,	2,	{0x96,0x05}},

	{0x00,	1,	{0x92}},
	{0xff,	2,	{0x10,0x02}},
	
	{0x00,	1,	{0x00}},
	{0x00,	1,	{0x00}},
	{0x00,	1,	{0x00}},
	{0x00,	1,	{0x00}},
	{0x00,	1,	{0x00}},
	{0x00,	1,	{0x00}},
	{0x00,	1,	{0x00}},
	{0x00,	1,	{0x00}},

	{0x00,	1,	{0x00}},
	{0xA0,	1,	{0x00}},
	
	{0x00,	1,	{0xA0}},
	{0xC1,	1,	{0x00}},
	{0x00,	1,	{0x80}},
	{0xC1,	2,	{0x36,0x66}},
	{0x00,	1,	{0x89}},
	{0xC0,	1,	{0x01}},
	{0x00,	1,	{0xB1}},
	{0xC5,	1,	{0x28}},
	{0x00,	1,	{0xC0}},
	{0xC5,	1,	{0x00}},
	{0x00,	1,	{0x80}},
       {0xC4,	1,	{0x9C}},
	
	{0x00,	1,	{0x90}},
	{0xC0,	6,	{0x00,0x44,0x00,0x00,0x00,0x03}},
	{0x00,	1,	{0xB4}},
	{0xC0,	1,	{0x10}},
	{0x00,	1,	{0xA6}},
	{0xC1,	3,	{0x00,0x00,0x00}},
	
	{0x00,	1,	{0x91}},
	{0xC5,	1,	{0x76}},
	{0x00,	1,	{0x93}},
	{0xC5,	1,	{0x76}},
	
	{0x00,	1,	{0xB2}},
	{0xF5,	4,	{0x15,0x00,0x15,0x00}},
	
	{0x00,	1,	{0x80}},
	{0xCB,	10,	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{0x00,	1,	{0x90}},
	{0xCB,	15,	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{0x00,	1,	{0xA0}},
	{0xCB,	15,	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{0x00,	1,	{0xB0}},
	{0xCB,	10,	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{0x00,	1,	{0xC0}},
	{0xCB,	15,	{0x00,0x00,0x00,0x04,0x00,0x00,0x04,0x04,0x00,0x00,0x04,0x04,0x04,0x00,0x00}},
	{0x00,	1,	{0xD0}},
	{0xCB,	15,	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x00,0x00,0x04,0x04}},
	{0x00,	1,	{0xE0}},
	{0xCB,	10,	{0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00}},
	{0x00,	1,	{0xF0}},
	{0xCB,	10,	{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}},
	{0x00,	1,	{0x80}},
	{0xCC,	10,	{0x00,0x00,0x00,0x02,0x00,0x00,0x0A,0x0E,0x00,0x00}},
	{0x00,	1,	{0x90}},
	{0xCC,	15,	{0x0C,0x10,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x09}},
	{0x00,	1,	{0xA0}},
	{0xCC,	15,	{0x0D,0x00,0x00,0x0B,0x0F,0x00,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x00}},
	{0x00,	1,	{0xB0}},
	{0xCC,	10,	{0x00,0x00,0x00,0x02,0x00,0x00,0x0A,0x0E,0x00,0x00}},
	{0x00,	1,	{0xC0}},
	{0xCC,	15,	{0x0C,0x10,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x09}},
	{0x00,	1,	{0xD0}},
	{0xCC,	15,	{0x0D,0x00,0x00,0x0B,0x0F,0x00,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x00}},
	{0x00,	1,	{0x80}},
	{0xCE,	12,	{0x85,0x03,0x18,0x84,0x03,0x18,0x00,0x0F,0x00,0x00,0x0F,0x00}},
	{0x00,	1,	{0x90}},
	{0xCE,	14,	{0x33,0xC5,0x18,0x33,0xC6,0x18,0xF0,0x00,0x00,0xF0,0x00,0x00,0x00,0x00}},
	{0x00,	1,	{0xA0}},
	{0xCE,	14,	{0x38,0x03,0x03,0xBF,0x00,0x18,0x00,0x38,0x02,0x03,0xC0,0x00,0x18,0x00}},
	{0x00,	1,	{0xB0}},
	{0xCE,	14,	{0x38,0x01,0x03,0xC1,0x00,0x18,0x00,0x38,0x00,0x03,0xC2,0x00,0x18,0x00}},
	{0x00,	1,	{0xC0}},
	{0xCE,	14,	{0x30,0x00,0x03,0xC3,0x00,0x18,0x00,0x30,0x01,0x03,0xC4,0x00,0x18,0x00}},			  
	{0x00,	1,	{0xD0}},
	{0xCE,	14,	{0x30,0x02,0x03,0xC5,0x00,0x18,0x00,0x30,0x03,0x03,0xC6,0x00,0x18,0x00}},
	{0x00,	1,	{0x80}},
	{0xCF,	14,	{0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00}},
	{0x00,	1,	{0x90}},
	{0xCF,	14,	{0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00}},
	{0x00,	1,	{0xA0}},
	{0xCF,	14,	{0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00}},
	{0x00,	1,	{0xB0}},
	{0xCF,	14,	{0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00}},
	{0x00,	1,	{0xC0}},
	{0xCF,	10,	{0x01,0x01,0x20,0x20,0x00,0x00,0x02,0x00,0x00,0x00}},
	
	//{0x00,	1,	{0x00}},
	//{0xD1,	2,	{0x00,0x00}},
	{0x00,	1,	{0x00}},
	{0xD8,	2,	{0x67,0x67}},
	{0x00,	1,	{0x00}},
	{0xD9,	1,	{0x62}},
	{0x00,	1,	{0x00}},
	{0xE1,	16,	{0x02,0x06,0x0A,0x0C,0x05,0x0C,0x0A,0x08,0x05,0x08,0x0F,0x09,0x10,0x18,0x12,0x08}},
	{0x00,	1,	{0x00}},
	{0xE2,	16,	{0x02,0x07,0x0A,0x0C,0x05,0x0B,0x0A,0x08,0x05,0x08,0x0F,0x09,0x10,0x18,0x12,0x08}},	

	{0x00,	1,	{0xB1}},
	{0xC5,	1,	{0x28}},
	{0x00,	1,	{0x80}},
	{0xC4,	1,	{0x9C}},
       {0x00,	1,	{0xC0}},
	{0xC5,	1,	{0x00}},
	{0x00,	1,	{0xB2}},
	{0xF5,	4,	{0x15,0x00,0x15,0x00}},
       {0x00,	1,	{0x93}},
	{0xC5,	3,	{0x03,0X55,0X55}},
	{0x00,	1,	{0x80}},
	{0xC1,	2,	{0x36,0x66}},
	{0x00,	1,	{0x89}},
	{0xC0,	1,	{0x01}},
	{0x00,	1,	{0xA0}},
	{0xC1,	1,	{0x02}},//00
	{0x00,	1,	{0xC5}},
	{0xB0,	1,	{0x03}},	
	{0x00,	1,	{0x00}},
	{0xFF,	3,	{0xFF,0xFF,0xFF}},
	{0x11,	1,	{0x00}},
	{REGFLAG_DELAY, 120, {}},
	{0x29,	1,	{0x00}},

	// Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.


	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
    {REGFLAG_DELAY,120, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

    // Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
static struct LCM_setting_table lcm_compare_id_setting[] = {
	// Display off sequence
	{0xF0,	5,	{0x55, 0xaa, 0x52,0x08,0x00}},
	{REGFLAG_DELAY, 10, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) 
{
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) 
{
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
				MDELAY(2);
       	}
    }
	
}


static void init_lcm_registers(void)
{
	unsigned int data_array[16];
}
// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = SYNC_PULSE_VDO_MODE;
#endif
	
		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_TWO_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
		params->dbi.io_driving_current      = LCM_DRIVING_CURRENT_4MA;
		// Video mode setting		
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		
		params->dsi.vertical_sync_active				= 1;// 3    2
		params->dsi.vertical_backporch					= 16;// 20   1
		params->dsi.vertical_frontporch					= 15; // 1  12
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 4;// 50  2
		params->dsi.horizontal_backporch				= 32;
		params->dsi.horizontal_frontporch				= 32;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		// Bit rate calculation
		params->dsi.PLL_CLOCK=210;
		params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		params->dsi.pll_div2=0;		// div2=0,1,2,3;div1_real=1,2,4,4	
#if (LCM_DSI_CMD_MODE)
		params->dsi.fbk_div =9;
#else
		params->dsi.fbk_div =9;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	
#endif
		//params->dsi.compatibility_for_nvk = 1;		// this parameter would be set to 1 if DriverIC is NTK's and when force match DSI clock for NTK's
}

static unsigned int lcm_compare_id(void)
{
		return 1;
		}

static void lcm_init(void)
{

	SET_RESET_PIN(0);
    MDELAY(200);
    SET_RESET_PIN(1);
    MDELAY(200);
	//lcm_compare_id();

    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
	
}



static void lcm_suspend(void)
{
		
			unsigned int data_array[16];
		
		data_array[0] = 0x00280500;
		dsi_set_cmdq(data_array, 1, 1);
		MDELAY(10); 

		data_array[0] = 0x00100500;
		dsi_set_cmdq(data_array, 1, 1);
		MDELAY(120);


}


static void lcm_resume(void)
{
		//lcm_init();
#ifdef BUILD_LK
		printf("zhibin uboot %s\n", __func__);
#else
		printk("zhibin kernel %s\n", __func__);
	
#endif
		push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);

}


static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];
	
#ifdef BUILD_LK
		printf("zhibin uboot %s\n", __func__);
#else
		printk("zhibin kernel %s\n", __func__);	
#endif

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(&data_array, 7, 0);

}

//#define ESD_DEBUG
static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_LK
    static int count = 0;
    static int err_count = 0;
    static int uncount = 0;
    int i;
    unsigned char fResult;
    unsigned char buffer[12];
    unsigned int array[16];

#ifdef ESD_DEBUG
    printk("lcm_esd_check <<<\n");
#endif
    for (i = 0; i < 12; i++)
        buffer[i] = 0x00;

    //---------------------------------
    // Set Maximum Return Size
    //---------------------------------
    //CKT_SET_HS_READ();
    array[0] = 0x00013708;
    dsi_set_cmdq(array, 1, 1);

    //---------------------------------
    // Read [9Ch, 00h, ECC] + Error Report(4 Bytes)
    //---------------------------------
    read_reg_v2(0x0A, buffer, 1);
    //CKT_RESTORE_HS_READ();

#ifdef ESD_DEBUG
    printk("lcm_esd_check : read(0x0A) : [0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x]\n", buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6]);
#endif

    //---------------------------------
    // Judge Readout & Error Report
    //---------------------------------
    if (buffer[3] == 0x02) // Check data identifier of error report
    {
        if (buffer[4] & 0x02) // Check SOT sync error
            err_count ++;
        else
            err_count = 0;
    }
    else
    {
        err_count = 0;
    }

#ifdef ESD_DEBUG
    printk("lcm_esd_check err_count = %d\n", err_count);
#endif
    if ((buffer[0] != 0x9C) || (err_count >= 2))
    {
        err_count = 0;
        uncount++;

#ifndef BUILD_LK
        printk("lcm_esd_check failed, err_count = %d\n", err_count);
        for (i = 0; i < 7; i++)
	        printk("buffer[%d] : 0x%x \n", i, buffer[i]);
#endif

#ifdef ESD_DEBUG
        printk("lcm_esd_check unnormal uncount = %d\n", uncount);
        printk("lcm_esd_check >>>\n");
#endif
        fResult = 1;
    }
    else
    {
        count ++;
#ifdef ESD_DEBUG
        printk("lcm_esd_check normal count = %d\n", count);
        printk("lcm_esd_check >>>\n");
#endif
        fResult = 0;
    }

#if 0
        printk("lcm_esd_check lcm_esd_test:%d\n",lcm_esd_test);
        if(lcm_esd_test==20)
        {
            lcm_esd_test = 0;
            return TRUE;
        }
		lcm_esd_test++;
	return FALSE;
#endif

    if (fResult)
        return TRUE;
    else
        return FALSE;
#endif
} 

static unsigned int lcm_esd_recover(void)
{
#ifndef BUILD_LK
    static int recount = 0;

#ifdef ESD_DEBUG
    printk("lcm_esd_recover\n");
#endif

    lcm_init();
    recount ++;


    //printk("lcm_esd_recover recover recount = %d\n", recount);

    return TRUE;
#endif
}

LCM_DRIVER otm9605a_dsi_vdo_dijing_lcm_drv = 
{
    .name			= "otm9605a_dsi_vdo_dijing",
	.set_util_funcs = lcm_set_util_funcs,
	.compare_id     = lcm_compare_id,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.esd_check   = lcm_esd_check,
	.esd_recover   = lcm_esd_recover,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
    };
