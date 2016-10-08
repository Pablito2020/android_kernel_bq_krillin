/* drivers/hwmon/mt6516/amit/tmd2772.c - TMD2772 ALS/PS driver
 * 
 * Author: MingHsien Hsieh <minghsien.hsieh@mediatek.com>
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
 */

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>
//#include <mach/mt_gpio.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>



#define POWER_NONE_MACRO MT65XX_POWER_NONE
#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include <asm/io.h>
#include <cust_eint.h>
#include <cust_alsps.h>
#include "tmd2772.h"
/******************************************************************************
 * configuration
*******************************************************************************/
/*----------------------------------------------------------------------------*/

#define TMD2772_DEV_NAME     "TMD2772"
/*----------------------------------------------------------------------------*/
#define APS_TAG                  "[ALS/PS] "
#define APS_FUN(f)               printk(APS_TAG"%s\n", __FUNCTION__)
#define APS_ERR(fmt, args...)    printk(KERN_ERR  APS_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define APS_LOG(fmt, args...)    printk(APS_TAG fmt, ##args)
#define APS_DBG(fmt, args...)    printk(APS_TAG fmt, ##args)                 
static u8  offset_data=0;
#define DO_CALIBARTION 1  
#define PRO_OFFSET 1
static u16 tmp_data=0;
#define OFFDATA_DEFAULT 1
/******************************************************************************
 * extern functions
*******************************************************************************/
/*for interrup work mode support --add by liaoxl.lenovo 12.08.2011*/
extern void mt_eint_mask(unsigned int eint_num);
extern void mt_eint_unmask(unsigned int eint_num);
extern void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
extern void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol);
extern unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens);
extern void mt_eint_registration(unsigned int eint_num, unsigned int flow, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
extern void mt_eint_print_status(void);

/*----------------------------------------------------------------------------*/
static struct i2c_client *tmd2772_i2c_client = NULL;
/*----------------------------------------------------------------------------*/
static const struct i2c_device_id tmd2772_i2c_id[] = {{TMD2772_DEV_NAME,0},{}};
static struct i2c_board_info __initdata i2c_TMD2772={ I2C_BOARD_INFO("TMD2772", (0X72>>1))};
/*the adapter id & i2c address will be available in customization*/
//static unsigned short tmd2772_force[] = {0x02, 0X72, I2C_CLIENT_END, I2C_CLIENT_END};
//static const unsigned short *const tmd2772_forces[] = { tmd2772_force, NULL };
//static struct i2c_client_address_data tmd2772_addr_data = { .forces = tmd2772_forces,};
/*----------------------------------------------------------------------------*/
static int tmd2772_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id); 
static int tmd2772_i2c_remove(struct i2c_client *client);
static int tmd2772_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);
/*----------------------------------------------------------------------------*/
static int tmd2772_i2c_suspend(struct i2c_client *client, pm_message_t msg);
static int tmd2772_i2c_resume(struct i2c_client *client);
static int store_status(unsigned int *flag);

static u8 store_enable_register=0;
extern struct alsps_hw *get_cust_alsps_hw(void);
	
static int  TMD2772_local_init(void);
static int  TMD_remove(void);
static int TMD2772_init_flag =-1; // 0<==>OK -1 <==> fail

static struct sensor_init_info TMD2772_init_info = {
		.name = "TMD2772",
		.init = TMD2772_local_init,
		.uninit = TMD_remove,
};
static struct tmd2772_priv *g_tmd2772_ptr = NULL;

static int tmd2772_init_client(struct i2c_client *client);
static void tmd2772_ps_calibrate(struct i2c_client *client);
static void tmd2772_ps_calibrate_call(struct i2c_client *client);
static int tmd2772_init_client_for_cali_call(struct i2c_client *client);
static int tmd2772_init_client_for_cali_restore(struct i2c_client *client);
static int en_ps = 1,ps_value;
int Enable_ALSPS_LOG = 0;
int cali_count = 13;
int test_cali=0;
int test_close=0;
int test_far=0;
u16 data_test[13]={0};
 int cali_num_end=0;
 int calling_first = 1;
 #define CKT_HALL_SWITCH_SUPPORT 0
 #if CKT_HALL_SWITCH_SUPPORT
extern int g_is_calling;
 #endif
 struct PS_CALI_DATA_STRUCT
{
    int close;
    int far_away;
    int valid;
} ;

static struct PS_CALI_DATA_STRUCT ps_cali={0,0,0};
static int intr_flag_value = 0;
static unsigned int temp_ps_data = 0;
static int indialing=0;

struct mutex mutex;

/*----------------------------------------------------------------------------*/
typedef enum {
    CMC_BIT_ALS    = 1,
    CMC_BIT_PS     = 2,
} CMC_BIT;
/*----------------------------------------------------------------------------*/
struct tmd2772_i2c_addr {    /*define a series of i2c slave address*/
    u8  write_addr;  
    u8  ps_thd;     /*PS INT threshold*/
};
/*----------------------------------------------------------------------------*/
struct tmd2772_priv {
    struct alsps_hw  *hw;
    struct i2c_client *client;
	struct work_struct  eint_work;
    //struct delayed_work  eint_work;

    /*i2c address group*/
    struct tmd2772_i2c_addr  addr;
    
    /*misc*/
    u16		    als_modulus;
    atomic_t    i2c_retry;
    atomic_t    als_suspend;
    atomic_t    als_debounce;   /*debounce time after enabling als*/
    atomic_t    als_deb_on;     /*indicates if the debounce is on*/
    atomic_t    als_deb_end;    /*the jiffies representing the end of debounce*/
    atomic_t    ps_mask;        /*mask ps: always return far away*/
    atomic_t    ps_debounce;    /*debounce time after enabling ps*/
    atomic_t    ps_deb_on;      /*indicates if the debounce is on*/
    atomic_t    ps_deb_end;     /*the jiffies representing the end of debounce*/
    atomic_t    ps_suspend;


    /*data*/
    u16         als;
    u16          ps;
    u8          _align;
    u16         als_level_num;
    u16         als_value_num;
    u32         als_level[C_CUST_ALS_LEVEL-1];
    u32         als_value[C_CUST_ALS_LEVEL];

    atomic_t    als_cmd_val;    /*the cmd value can't be read, stored in ram*/
    atomic_t    ps_cmd_val;     /*the cmd value can't be read, stored in ram*/
    atomic_t    ps_thd_val_high;     /*the cmd value can't be read, stored in ram*/
	atomic_t    ps_thd_val_low;     /*the cmd value can't be read, stored in ram*/
    ulong       enable;         /*enable mask*/
    ulong       pending_intr;   /*pending interrupt*/

    /*early suspend*/
#if defined(CONFIG_HAS_EARLYSUSPEND)
    struct early_suspend    early_drv;
#endif     
};
/*----------------------------------------------------------------------------*/
static struct i2c_driver tmd2772_i2c_driver = {	
	.probe      = tmd2772_i2c_probe,
	.remove     = tmd2772_i2c_remove,
	.suspend    = tmd2772_i2c_suspend,
	.resume     = tmd2772_i2c_resume,
	.id_table   = tmd2772_i2c_id,
//	.address_data = &tmd2772_addr_data,
	.driver = {
//		.owner          = THIS_MODULE,
		.name           = TMD2772_DEV_NAME,
	},
};

static int tmd2772_read_data_for_cali(struct i2c_client *client, struct PS_CALI_DATA_STRUCT *ps_data_cali);
static struct tmd2772_priv *tmd2772_obj = NULL;
static struct platform_driver tmd2772_alsps_driver;
/*----------------------------------------------------------------------------*/
int tmd2772_get_addr(struct alsps_hw *hw, struct tmd2772_i2c_addr *addr)
{
	if(!hw || !addr)
	{
		return -EFAULT;
	}
	addr->write_addr= hw->i2c_addr[0];
	return 0;
}
/*----------------------------------------------------------------------------*/
static void tmd2772_power(struct alsps_hw *hw, unsigned int on) 
{
	static unsigned int power_on = 0;

	//APS_LOG("power %s\n", on ? "on" : "off");

	if(hw->power_id != POWER_NONE_MACRO)
	{
		if(power_on == on)
		{
			APS_LOG("ignore power control: %d\n", on);
		}
		else if(on)
		{
			if(!hwPowerOn(hw->power_id, hw->power_vol, "TMD2772")) 
			{
				APS_ERR("power on fails!!\n");
			}
		}
		else
		{
			if(!hwPowerDown(hw->power_id, "TMD2772")) 
			{
				APS_ERR("power off fail!!\n");   
			}
		}
	}
	power_on = on;
}
/*----------------------------------------------------------------------------*/
static long tmd2772_enable_als(struct i2c_client *client, int enable)
{
		struct tmd2772_priv *obj = i2c_get_clientdata(client);
		u8 databuf[2];	  
		long res = 0;
		//u8 buffer[1];
		//u8 reg_value[1];
		uint32_t testbit_PS;
		
	
		if(client == NULL)
		{
			APS_DBG("CLIENT CANN'T EQUL NULL\n");
			return -1;
		}
		
		#if 0	/*yucong MTK enable_als function modified for fixing reading register error problem 2012.2.16*/
		buffer[0]=TMD2772_CMM_ENABLE;
		res = i2c_master_send(client, buffer, 0x1);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		res = i2c_master_recv(client, reg_value, 0x1);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		printk("Yucong:0x%x, %d, %s\n", reg_value[0], __LINE__, __FUNCTION__);
		
		if(enable)
		{
			databuf[0] = TMD2772_CMM_ENABLE;	
			databuf[1] = reg_value[0] |0x0B;
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
			/*Lenovo-sw chenlj2 add 2011-06-03,modify ps to ALS below two lines */
			atomic_set(&obj->als_deb_on, 1);
			atomic_set(&obj->als_deb_end, jiffies+atomic_read(&obj->als_debounce)/(1000/HZ));
			APS_DBG("tmd2772 power on\n");
		}
		else
		{
			databuf[0] = TMD2772_CMM_ENABLE;	
			databuf[1] = reg_value[0] &0xFD;
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
			/*Lenovo-sw chenlj2 add 2011-06-03,modify ps_deb_on to als_deb_on */
			atomic_set(&obj->als_deb_on, 0);
			APS_DBG("tmd2772 power off\n");
		}
		#endif
		#if 1
		/*yucong MTK enable_als function modified for fixing reading register error problem 2012.2.16*/
		testbit_PS = test_bit(CMC_BIT_PS, &obj->enable) ? (1) : (0);
		if(enable)
		{
			if(testbit_PS){	
			databuf[0] = TMD2772_CMM_ENABLE;	
			databuf[1] = 0x2F;
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
				{
					goto EXIT_ERR;
				}
			/*debug code for reading register value*/
			#if 0
			res = i2c_master_recv(client, reg_value, 0x1);
			if(res <= 0)
				{
					goto EXIT_ERR;
				}
			printk("Yucong:0x%x, %d, %s\n", reg_value[0], __LINE__, __FUNCTION__);
			#endif
			}
			else{
			databuf[0] = TMD2772_CMM_ENABLE;	
			databuf[1] = 0x2B;
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
				{
					goto EXIT_ERR;
				}

			/*debug code for reading register value*/
			#if 0
			res = i2c_master_recv(client, reg_value, 0x1);
			if(res <= 0)
				{
					goto EXIT_ERR;
				}
			printk("Yucong:0x%x, %d, %s\n", reg_value[0], __LINE__, __FUNCTION__);
			#endif

			}
			atomic_set(&obj->als_deb_on, 1);
			atomic_set(&obj->als_deb_end, jiffies+atomic_read(&obj->als_debounce)/(1000/HZ));
			APS_DBG("tmd2772 power on\n");
		}
		else
		{	
			if(testbit_PS){
			databuf[0] = TMD2772_CMM_ENABLE;	
			databuf[1] = 0x2D;
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
				{
					goto EXIT_ERR;
				}
			}
			else{
			databuf[0] = TMD2772_CMM_ENABLE;	
			databuf[1] = 0x00;
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
				{
					goto EXIT_ERR;
				}
			}
			/*Lenovo-sw chenlj2 add 2011-06-03,modify ps_deb_on to als_deb_on */
			atomic_set(&obj->als_deb_on, 0);
			APS_DBG("tmd2772 power off\n");
		}
		#endif
		#if 0 /*yucong add for debug*/
			buffer[0]=TMD2772_CMM_ENABLE;
			res = i2c_master_send(client, buffer, 0x1);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
			res = i2c_master_recv(client, reg_value, 0x1);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
			printk("Yucong:0x%x, %d, %s\n", reg_value[0], __LINE__, __FUNCTION__);
		#endif
		
		return 0;
		
	EXIT_ERR:
		APS_ERR("tmd2772_enable_als fail\n");
		return res;
}

/*----------------------------------------------------------------------------*/
static long tmd2772_enable_ps(struct i2c_client *client, int enable)
{
	struct tmd2772_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];    
	long res = 0;
//	u8 buffer[1];
//	u8 reg_value[1];
	uint32_t testbit_ALS;

	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}
	/*yucong MTK: lenovo orignal code*/
	printk("luosenalsps tmd2772_enable_ps = %d,\n",enable);


#if 1	
	/*yucong MTK: enable_ps function modified for fixing reading register error problem 2012.2.16*/
	testbit_ALS = test_bit(CMC_BIT_ALS, &obj->enable) ? (1) : (0);
	if(enable)
	{
		if(testbit_ALS){
		databuf[0] = TMD2772_CMM_ENABLE;    
		databuf[1] = 0x0F;
		res = i2c_master_send(client, databuf, 0x2);
		if(res <= 0)
			{
				goto EXIT_ERR;
			}
		/*debug code for reading register value*/
		#if 0
		res = i2c_master_recv(client, reg_value, 0x1);
		if(res <= 0)
			{
				goto EXIT_ERR;
			}
		printk("Yucong:0x%x, %d, %s\n", reg_value[0], __LINE__, __FUNCTION__);
		#endif
		}else{
		databuf[0] = TMD2772_CMM_ENABLE;    
		databuf[1] = 0x0D;
		res = i2c_master_send(client, databuf, 0x2);
		if(res <= 0)
			{
				goto EXIT_ERR;
			}
		}
		/*debug code for reading register value*/
		#if 0
		res = i2c_master_recv(client, reg_value, 0x1);
		if(res <= 0)
			{
				goto EXIT_ERR;
			}
		printk("Yucong:0x%x, %d, %s\n", reg_value[0], __LINE__, __FUNCTION__);
		#endif
		atomic_set(&obj->ps_deb_on, 1);
		atomic_set(&obj->ps_deb_end, jiffies+atomic_read(&obj->ps_debounce)/(1000/HZ));
		APS_DBG("tmd2772 power on\n");
		 #if CKT_HALL_SWITCH_SUPPORT
              g_is_calling=1;
               #endif

		/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
		if(0 == obj->hw->polling_mode_ps)
		{
			if(1 == ps_cali.valid)
			{
				databuf[0] = TMD2772_CMM_INT_LOW_THD_LOW;	
				databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_low) & 0x00FF);
				res = i2c_master_send(client, databuf, 0x2);
				if(res <= 0)
				{
					goto EXIT_ERR;
					return TMD2772_ERR_I2C;
				}
				databuf[0] = TMD2772_CMM_INT_LOW_THD_HIGH;	
				databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_low) & 0xFF00) >> 8);
				res = i2c_master_send(client, databuf, 0x2);
				if(res <= 0)
				{
					goto EXIT_ERR;
					return TMD2772_ERR_I2C;
				}
				databuf[0] = TMD2772_CMM_INT_HIGH_THD_LOW;	
				databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_high) & 0x00FF);
				res = i2c_master_send(client, databuf, 0x2);
				if(res <= 0)
				{
					goto EXIT_ERR;
					return TMD2772_ERR_I2C;
				}
				databuf[0] = TMD2772_CMM_INT_HIGH_THD_HIGH; 
				databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_high) & 0xFF00) >> 8);;
				res = i2c_master_send(client, databuf, 0x2);
				if(res <= 0)
				{
					goto EXIT_ERR;
					return TMD2772_ERR_I2C;
				}
			}
			else
			{
				databuf[0] = TMD2772_CMM_INT_LOW_THD_LOW;	
				databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_low) & 0x00FF);
				res = i2c_master_send(client, databuf, 0x2);
				if(res <= 0)
				{
					goto EXIT_ERR;
					return TMD2772_ERR_I2C;
				}
				databuf[0] = TMD2772_CMM_INT_LOW_THD_HIGH;	
				databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_low) & 0xFF00) >> 8);
				res = i2c_master_send(client, databuf, 0x2);
				if(res <= 0)
				{
					goto EXIT_ERR;
					return TMD2772_ERR_I2C;
				}
				databuf[0] = TMD2772_CMM_INT_HIGH_THD_LOW;	
				databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_high) & 0x00FF);
				res = i2c_master_send(client, databuf, 0x2);
				if(res <= 0)
				{
					goto EXIT_ERR;
					return TMD2772_ERR_I2C;
				}
				databuf[0] = TMD2772_CMM_INT_HIGH_THD_HIGH; 
				databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_high) & 0xFF00) >> 8);
				res = i2c_master_send(client, databuf, 0x2);
				if(res <= 0)
				{
					goto EXIT_ERR;
					return TMD2772_ERR_I2C;
				}
		
			}
		
			databuf[0] = TMD2772_CMM_Persistence;
			databuf[1] = 0x20;
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
			{
				goto EXIT_ERR;
				return TMD2772_ERR_I2C;
			}
			if(testbit_ALS){
			databuf[0] = TMD2772_CMM_ENABLE;    
			databuf[1] = 0x2F;
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
				{
					goto EXIT_ERR;
				}
			/*debug code for reading register value*/
			#if 0
			res = i2c_master_recv(client, reg_value, 0x1);
			if(res <= 0)
				{
					goto EXIT_ERR;
				}
			printk("Yucong:0x%x, %d, %s\n", reg_value[0], __LINE__, __FUNCTION__);
			#endif
			}else{
			databuf[0] = TMD2772_CMM_ENABLE;    
			databuf[1] = 0x2D;
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
				{
					goto EXIT_ERR;
				}
			}
			/*debug code for reading register value*/
			#if 0
			res = i2c_master_recv(client, reg_value, 0x1);
			if(res <= 0)
				{
					goto EXIT_ERR;
				}
			printk("Yucong:0x%x, %d, %s\n", reg_value[0], __LINE__, __FUNCTION__);
			#endif
		
			mt_eint_unmask(CUST_EINT_ALS_NUM);
		}
	}
	else
	{
	/*yucong MTK: enable_ps function modified for fixing reading register error problem 2012.2.16*/
	if(testbit_ALS){
		databuf[0] = TMD2772_CMM_ENABLE;    
		databuf[1] = 0x2B;
		res = i2c_master_send(client, databuf, 0x2);
		if(res <= 0)
			{
				goto EXIT_ERR;
			}
		}else{
		databuf[0] = TMD2772_CMM_ENABLE;    
		databuf[1] = 0x00;
		res = i2c_master_send(client, databuf, 0x2);
		if(res <= 0)
			{
				goto EXIT_ERR;
			}
		}
		atomic_set(&obj->ps_deb_on, 0);
		APS_DBG("tmd2772 power off\n");
		#if CKT_HALL_SWITCH_SUPPORT
              g_is_calling=0;
               #endif

		/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
		if(0 == obj->hw->polling_mode_ps)
		{
			cancel_work_sync(&obj->eint_work);
			mt_eint_mask(CUST_EINT_ALS_NUM);
		}
	}
#endif
	return 0;
	
EXIT_ERR:
	APS_ERR("tmd2772_enable_ps fail\n");
	return res;
}
/*----------------------------------------------------------------------------*/
#if 0
static int tmd2772_enable(struct i2c_client *client, int enable)
{
	struct tmd2772_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];    
	int res = 0;
	u8 buffer[1];
	u8 reg_value[1];

	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}

	/* modify to restore reg setting after cali ---liaoxl.lenovo */
	buffer[0]=TMD2772_CMM_ENABLE;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, reg_value, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

	if(enable)
	{
		databuf[0] = TMD2772_CMM_ENABLE;    
		databuf[1] = reg_value[0] | 0x01;
		res = i2c_master_send(client, databuf, 0x2);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		APS_DBG("tmd2772 power on\n");
	}
	else
	{
		databuf[0] = TMD2772_CMM_ENABLE;    
		databuf[1] = reg_value[0] & 0xFE;
		res = i2c_master_send(client, databuf, 0x2);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		atomic_set(&obj->ps_deb_on, 0);
		/*Lenovo-sw chenlj2 add 2011-06-03,close als_deb_on */
		atomic_set(&obj->als_deb_on, 0);
		APS_DBG("tmd2772 power off\n");
	}
	return 0;
	
EXIT_ERR:
	APS_ERR("tmd2772_enable fail\n");
	return res;
}
#endif

/*----------------------------------------------------------------------------*/
/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
static int tmd2772_check_and_clear_intr(struct i2c_client *client) 
{
	//struct tmd2772_priv *obj = i2c_get_clientdata(client);
	int res,intp,intl;
	u8 buffer[2];

	//if (mt_get_gpio_in(GPIO_ALS_EINT_PIN) == 1) /*skip if no interrupt*/  
	//    return 0;

	buffer[0] = TMD2772_CMM_STATUS;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	//printk("yucong tmd2772_check_and_clear_intr status=0x%x\n", buffer[0]);
	res = 1;
	intp = 0;
	intl = 0;
	if(0 != (buffer[0] & 0x20))
	{
		res = 0;
		intp = 1;
	}
	if(0 != (buffer[0] & 0x10))
	{
		res = 0;
		intl = 1;		
	}

	if(0 == res) 
	{
		if((1 == intp) && (0 == intl))
		{
			buffer[0] = (TAOS_TRITON_CMD_REG|TAOS_TRITON_CMD_SPL_FN|0x05);
		}
		else if((0 == intp) && (1 == intl))
		{
			buffer[0] = (TAOS_TRITON_CMD_REG|TAOS_TRITON_CMD_SPL_FN|0x06);
		}
		else
		{
			buffer[0] = (TAOS_TRITON_CMD_REG|TAOS_TRITON_CMD_SPL_FN|0x07);
		}
		res = i2c_master_send(client, buffer, 0x1);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		else
		{
			res = 0;
		}
	}

	return res;

EXIT_ERR:
	APS_ERR("tmd2772_check_and_clear_intr fail\n");
	return 1;
}
/*----------------------------------------------------------------------------*/

/*yucong add for interrupt mode support MTK inc 2012.3.7*/
static int tmd2772_check_intr(struct i2c_client *client) 
{
//	struct tmd2772_priv *obj = i2c_get_clientdata(client);
	int res,intp,intl;
	u8 buffer[2];

	//if (mt_get_gpio_in(GPIO_ALS_EINT_PIN) == 1) /*skip if no interrupt*/  
	//    return 0;

	buffer[0] = TMD2772_CMM_STATUS;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	//APS_ERR("tmd2772_check_and_clear_intr status=0x%x\n", buffer[0]);
	res = 1;
	intp = 0;
	intl = 0;
	if(0 != (buffer[0] & 0x20))
	{
		res = 0;
		intp = 1;
	}
	if(0 != (buffer[0] & 0x10))
	{
		res = 0;
		intl = 1;		
	}

	return res;

EXIT_ERR:
	APS_ERR("tmd2772_check_intr fail\n");
	return 1;
}

static int tmd2772_clear_intr(struct i2c_client *client) 
{
//	struct tmd2772_priv *obj = i2c_get_clientdata(client);
	int res;
	u8 buffer[2];

#if 0
	if((1 == intp) && (0 == intl))
	{
		buffer[0] = (TAOS_TRITON_CMD_REG|TAOS_TRITON_CMD_SPL_FN|0x05);
	}
	else if((0 == intp) && (1 == intl))
	{
		buffer[0] = (TAOS_TRITON_CMD_REG|TAOS_TRITON_CMD_SPL_FN|0x06);
	}
	else
#endif
	{
		buffer[0] = (TAOS_TRITON_CMD_REG|TAOS_TRITON_CMD_SPL_FN|0x07);
	}
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	else
	{
		res = 0;
	}

	return res;

EXIT_ERR:
	APS_ERR("tmd2772_check_and_clear_intr fail\n");
	return 1;
}


/*-----------------------------------------------------------------------------*/
void tmd2772_eint_func(void)
{
	APS_FUN();

	struct tmd2772_priv *obj = g_tmd2772_ptr;
	if(!obj)
	{
		return;
	}
	
	schedule_work(&obj->eint_work);
	//schedule_delayed_work(&obj->eint_work);
}

/*----------------------------------------------------------------------------*/
/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
int tmd2772_setup_eint(struct i2c_client *client)
{
	struct tmd2772_priv *obj = i2c_get_clientdata(client);        

	g_tmd2772_ptr = obj;
	
	mt_set_gpio_dir(GPIO_ALS_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_mode(GPIO_ALS_EINT_PIN, GPIO_ALS_EINT_PIN_M_EINT);
	mt_set_gpio_pull_enable(GPIO_ALS_EINT_PIN, TRUE);
	mt_set_gpio_pull_select(GPIO_ALS_EINT_PIN, GPIO_PULL_UP);

	mt_eint_set_hw_debounce(CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_CN);
	mt_eint_registration(CUST_EINT_ALS_NUM, CUST_EINT_ALS_TYPE, tmd2772_eint_func, 0);

	mt_eint_unmask(CUST_EINT_ALS_NUM); 
    return 0;
}

/*----------------------------------------------------------------------------*/

#if 1
static int tmd2772_init_client_for_cali_call(struct i2c_client *client)
{
	struct tmd2772_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];    
	int res = 0;
	databuf[0] = TMD2772_CMM_ENABLE;    
	databuf[1] = 0x01;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
	databuf[0] = TMD2772_CMM_ATIME;    
	databuf[1] = 0xff;//0xEE
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
	databuf[0] = TMD2772_CMM_PTIME;    
	databuf[1] = 0xFF;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
	databuf[0] = TMD2772_CMM_WTIME;    
	databuf[1] = 0xFF;//0xFF
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
	databuf[0] = TMD2772_CMM_CONFIG;    
	databuf[1] = 0x00;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
	databuf[0] = TMD2772_CMM_PPCOUNT;    
	databuf[1] = TMD2772_CMM_PPCOUNT_VALUE;//0x02
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
	databuf[0] = TMD2772_CMM_CONTROL;    
	databuf[1] = TMD2772_CMM_CONTROL_VALUE;//0x22
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
#if DO_CALIBARTION	
  #if PRO_OFFSET
	databuf[0] = TMD2772_CMM_OFFSET;    
	databuf[1] = offset_data;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
  #endif
#endif
	databuf[0] = TMD2772_CMM_ENABLE;	
	databuf[1] = 0x05;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
	return TMD2772_SUCCESS;
EXIT_ERR:
	APS_ERR("init dev: %d\n", res);
	return res;
}
static int tmd2772_init_client_for_cali(struct i2c_client *client)
{

	struct tmd2772_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];    
	int res = 0;
   
	databuf[0] = TMD2772_CMM_ENABLE;    
	databuf[1] = 0x01;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
	
	databuf[0] = TMD2772_CMM_ATIME;    
	databuf[1] = 0xEE;//0xEE
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}

	databuf[0] = TMD2772_CMM_PTIME;    
	databuf[1] = 0xFF;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}

	databuf[0] = TMD2772_CMM_WTIME;    
	databuf[1] = 0xFF;//0xFF
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}

	databuf[0] = TMD2772_CMM_CONFIG;    
	databuf[1] = 0x00;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}

	databuf[0] = TMD2772_CMM_PPCOUNT;    
	databuf[1] = TMD2772_CMM_PPCOUNT_VALUE;//0x02
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}

	databuf[0] = TMD2772_CMM_CONTROL;    
	databuf[1] = TMD2772_CMM_CONTROL_VALUE;//0x22
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
#if DO_CALIBARTION
  #if PRO_OFFSET
	databuf[0] = TMD2772_CMM_OFFSET;
	databuf[1] = 0x00;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
  #endif	
  #else
	databuf[0] = TMD2772_CMM_OFFSET;    
	databuf[1] = 0x1F;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
#endif
	databuf[0] = TMD2772_CMM_ENABLE;	
		databuf[1] = 0x0F;
		res = i2c_master_send(client, databuf, 0x2);
		if(res <= 0)
		{
			goto EXIT_ERR;
			return TMD2772_ERR_I2C;
		}

	return TMD2772_SUCCESS;

EXIT_ERR:
	APS_ERR("init dev: %d\n", res);
	return res;

}
#endif

static int tmd2772_init_client(struct i2c_client *client)
{
	struct tmd2772_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];    
	int res = 0;
   
	databuf[0] = TMD2772_CMM_ENABLE;    
	databuf[1] = 0x00;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
	
	databuf[0] = TMD2772_CMM_ATIME;    
	databuf[1] = 0xEE;//0xF6;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}

	databuf[0] = TMD2772_CMM_PTIME;    
	databuf[1] = 0xFF;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}

	databuf[0] = TMD2772_CMM_WTIME;    
	databuf[1] = 0xEE;//0xFC;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
	/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
	if(0 == obj->hw->polling_mode_ps)
	{
		if(1 == ps_cali.valid)
		{
			databuf[0] = TMD2772_CMM_INT_LOW_THD_LOW;	
			//databuf[1] = (u8)(ps_cali.far_away & 0x00FF);
			    databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_low) & 0x00FF);
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
			{
				goto EXIT_ERR;
				return TMD2772_ERR_I2C;
			}
			databuf[0] = TMD2772_CMM_INT_LOW_THD_HIGH;	
			databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_low) & 0xFF00) >> 8);
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
			{
				goto EXIT_ERR;
				return TMD2772_ERR_I2C;
			}
			databuf[0] = TMD2772_CMM_INT_HIGH_THD_LOW;	
			databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_high) & 0x00FF);
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
			{
				goto EXIT_ERR;
				return TMD2772_ERR_I2C;
			}
			databuf[0] = TMD2772_CMM_INT_HIGH_THD_HIGH;	
			databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_high) & 0xFF00) >> 8);;
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
			{
				goto EXIT_ERR;
				return TMD2772_ERR_I2C;
			}
		}
		else
		{
			databuf[0] = TMD2772_CMM_INT_LOW_THD_LOW;	
			databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_low) & 0x00FF);
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
			{
				goto EXIT_ERR;
				return TMD2772_ERR_I2C;
			}
			databuf[0] = TMD2772_CMM_INT_LOW_THD_HIGH;	
			databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_low) & 0xFF00) >> 8);
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
			{
				goto EXIT_ERR;
				return TMD2772_ERR_I2C;
			}
			databuf[0] = TMD2772_CMM_INT_HIGH_THD_LOW;	
			databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_high) & 0x00FF);
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
			{
				goto EXIT_ERR;
				return TMD2772_ERR_I2C;
			}
			databuf[0] = TMD2772_CMM_INT_HIGH_THD_HIGH;	
			databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_high) & 0xFF00) >> 8);
			res = i2c_master_send(client, databuf, 0x2);
			if(res <= 0)
			{
				goto EXIT_ERR;
				return TMD2772_ERR_I2C;
			}

		}

		databuf[0] = TMD2772_CMM_Persistence;
		databuf[1] = 0x20;
		res = i2c_master_send(client, databuf, 0x2);
		if(res <= 0)
		{
			goto EXIT_ERR;
			return TMD2772_ERR_I2C;
		}
		databuf[0] = TMD2772_CMM_ENABLE;	
		databuf[1] = 0x20;
		res = i2c_master_send(client, databuf, 0x2);
		if(res <= 0)
		{
			goto EXIT_ERR;
			return TMD2772_ERR_I2C;
		}

	}

	databuf[0] = TMD2772_CMM_CONFIG;    
	databuf[1] = 0x00;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}

       /*Lenovo-sw chenlj2 add 2011-06-03,modified pulse 2  to 4 */
	databuf[0] = TMD2772_CMM_PPCOUNT;    
	databuf[1] = TMD2772_CMM_PPCOUNT_VALUE;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}

        /*Lenovo-sw chenlj2 add 2011-06-03,modified gain 16  to 1 */
	databuf[0] = TMD2772_CMM_CONTROL;    
	databuf[1] = TMD2772_CMM_CONTROL_VALUE;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
#if 	0
	databuf[0] = TMD2772_CMM_ID;  
	res = i2c_master_send(client, databuf, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
	res = i2c_master_recv(client, databuf, 0x1);
	if(res <= 0)
	{		
	    goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
    printk("ALSPS ID: %x\n",databuf[0]);//0x30 or 0x39
   #endif
   #if DO_CALIBARTION
   #if PRO_OFFSET
     databuf[0] = TMD2772_CMM_OFFSET;
     databuf[1] = offset_data;
     res = i2c_master_send(client, databuf, 0x2);
     if(res <= 0)
     {
    	goto EXIT_ERR;
    	return TMD2772_ERR_I2C;
     }
  #endif
#else
	databuf[0] = TMD2772_CMM_OFFSET;    
	databuf[1] = 0x00;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
#endif
	/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
	if(0 == obj->hw->polling_mode_ps)
	{
	if((res = tmd2772_setup_eint(client))!=0)
	{
		APS_ERR("setup eint: %d\n", res);
		return res;
	}
	
	if((res = tmd2772_check_and_clear_intr(client)))
	{
		APS_ERR("check/clear intr: %d\n", res);
		//    return res;
	}
	}
	return TMD2772_SUCCESS;

EXIT_ERR:
	APS_ERR("tmd2772 init dev fail: %d\n", res);
	return res;
}

/****************************************************************************** 
 * Function Configuration
******************************************************************************/
int tmd2772_read_als(struct i2c_client *client, u16 *data)
{
	struct tmd2772_priv *obj = i2c_get_clientdata(client);	 
	u16 c0_value, c1_value;	 
	u32 c0_nf, c1_nf;
	u8 als_value_low[1], als_value_high[1];
	u8 buffer[1];
	u16 atio;
	//u16 als_value;
	int res = 0;
//	u8 reg_value[1];
	
	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}

	/*debug tag for yucong*/
	#if 0
	buffer[0]=TMD2772_CMM_ENABLE;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, reg_value, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	printk("Yucong:0x%x, %d, %s\n", reg_value[0], __LINE__, __FUNCTION__);
	#endif
//get adc channel 0 value
	buffer[0]=TMD2772_CMM_C0DATA_L;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, als_value_low, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	//printk("yucong: TMD2772_CMM_C0DATA_L = 0x%x\n", als_value_low[0]);

	buffer[0]=TMD2772_CMM_C0DATA_H;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, als_value_high, 0x01);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	//printk("yucong: TMD2772_CMM_C0DATA_H = 0x%x\n", als_value_high[0]);
	c0_value = als_value_low[0] | (als_value_high[0]<<8);
	c0_nf = obj->als_modulus*c0_value;
	APS_DBG("c0_value=%d, c0_nf=%d, als_modulus=%d\n", c0_value, c0_nf, obj->als_modulus);

//get adc channel 1 value
	buffer[0]=TMD2772_CMM_C1DATA_L;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, als_value_low, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	//printk("yucong: TMD2772_CMM_C1DATA_L = 0x%x\n", als_value_low[0]);	

	buffer[0]=TMD2772_CMM_C1DATA_H;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, als_value_high, 0x01);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	//printk("yucong: TMD2772_CMM_C1DATA_H = 0x%x\n", als_value_high[0]);	

	c1_value = als_value_low[0] | (als_value_high[0]<<8);
	c0_nf = obj->als_modulus*c0_value;
	c1_nf = obj->als_modulus*c1_value;	
	APS_DBG("c1_value=%d, c1_nf=%d, als_modulus=%d\n", c1_value, c1_nf, obj->als_modulus);

	if((c0_value > c1_value) &&(c0_value < 50000))
	{  	/*Lenovo-sw chenlj2 add 2011-06-03,add {*/
		atio = (c1_nf*100)/c0_nf;

	APS_DBG("atio = %d\n", atio);
	if(atio<30)
	{
		*data = (13*c0_nf - 24*c1_nf)/10000;
	}
	else if(atio>= 30 && atio<38) /*Lenovo-sw chenlj2 add 2011-06-03,modify > to >=*/
	{ 
		*data = (16*c0_nf - 35*c1_nf)/10000;
	}
	else if(atio>= 38 && atio<45)  /*Lenovo-sw chenlj2 add 2011-06-03,modify > to >=*/
	{ 
		*data = (9*c0_nf - 17*c1_nf)/10000;
	}
	else if(atio>= 45 && atio<54) /*Lenovo-sw chenlj2 add 2011-06-03,modify > to >=*/
	{ 
		*data = (6*c0_nf - 10*c1_nf)/10000;
	}
	else
		*data = 0;
	/*Lenovo-sw chenlj2 add 2011-06-03,add }*/
    }
	else if (c0_value > 50000)
	{
		*data = 65535;
	}
	else if(c0_value == 0)
        {
                *data = 0;
        }
        else
	{
		APS_DBG("als_value is invalid!!\n");
		return -1;
	}	
	APS_DBG("als_value_lux = %d\n", *data);
	//printk("yucong: als_value_lux = %d\n", *data);
	return 0;	 

	
	
EXIT_ERR:
	APS_ERR("tmd2772_read_ps fail\n");
	return res;
}
int tmd2772_read_als_ch0(struct i2c_client *client, u16 *data)
{
//	struct tmd2772_priv *obj = i2c_get_clientdata(client);	 
	u16 c0_value;	 
	u8 als_value_low[1], als_value_high[1];
	u8 buffer[1];
	int res = 0;
	
	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}
//get adc channel 0 value
	buffer[0]=TMD2772_CMM_C0DATA_L;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, als_value_low, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	buffer[0]=TMD2772_CMM_C0DATA_H;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, als_value_high, 0x01);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	c0_value = als_value_low[0] | (als_value_high[0]<<8);
	*data = c0_value;
	return 0;	 

	
	
EXIT_ERR:
	APS_ERR("tmd2772_read_ps fail\n");
	return res;
}
/*----------------------------------------------------------------------------*/

static int tmd2772_get_als_value(struct tmd2772_priv *obj, u16 als)
{
	int idx;
	int invalid = 0;
	for(idx = 0; idx < obj->als_level_num; idx++)
	{
		if(als < obj->hw->als_level[idx])
		{
			break;
		}
	}
	
	if(idx >= obj->als_value_num)
	{
		APS_ERR("exceed range\n"); 
		idx = obj->als_value_num - 1;
	}
	
	if(1 == atomic_read(&obj->als_deb_on))
	{
		unsigned long endt = atomic_read(&obj->als_deb_end);
		if(time_after(jiffies, endt))
		{
			atomic_set(&obj->als_deb_on, 0);
		}
		
		if(1 == atomic_read(&obj->als_deb_on))
		{
			invalid = 1;
		}
	}

	if(!invalid)
	{
		APS_DBG("ALS: raw data %05d => value = %05d\n", als, obj->hw->als_value[idx]);	
		return obj->hw->als_value[idx];
	}
	else
	{
		APS_ERR("ALS: %05d => %05d (-1)\n", als, obj->hw->als_value[idx]);    
		return -1;
	}
}
/*----------------------------------------------------------------------------*/
long tmd2772_read_ps(struct i2c_client *client, u16 *data)
{
//	struct tmd2772_priv *obj = i2c_get_clientdata(client);    
//	u16 ps_value;    
	u8 ps_value_low[1], ps_value_high[1];
	u8 buffer[1];
	u8 buffer_id[1]={0};
	long res = 0;

	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}
       #if 0
	buffer[0]=0x92;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, buffer_id, 0x01);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	#endif
	buffer[0]=TMD2772_CMM_PDATA_L;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, ps_value_low, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

	buffer[0]=TMD2772_CMM_PDATA_H;
	res = i2c_master_send(client, buffer, 0x1);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = i2c_master_recv(client, ps_value_high, 0x01);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

	if((*data = (ps_value_low[0] | (ps_value_high[0]<<8))) > 1023)
		*data = tmp_data;
	else
		tmp_data = *data;
	if(1==Enable_ALSPS_LOG)
		{
	printk("luosen --read_ps_data =%x\n",*data);
	printk("luosen read_ps_data=%d, low:%d  high:%d\n", *data, ps_value_low[0], ps_value_high[0]);
              }
	return 0;    

EXIT_ERR:
	APS_ERR("tmd2772_read_ps fail\n");
	return res;
}
/*----------------------------------------------------------------------------*/
static int tmd2772_get_ps_value(struct tmd2772_priv *obj, u16 ps)
{
	int val,mask = atomic_read(&obj->ps_mask);

	int invalid = 0;
	int j;
	static int val_temp=1;
	 /*Lenovo-sw chenlj2 add 2011-10-12 begin*/
	 u16 temp_ps[1];
	 /*Lenovo-sw chenlj2 add 2011-10-12 end*/
	 
	
	//APS_LOG("tmd2772_get_ps_value  1 %d," ,ps_cali.close);
	//APS_LOG("tmd2772_get_ps_value  2 %d," ,ps_cali.far_away);
	//APS_LOG("tmd2772_get_ps_value  3 %d,", ps_cali.valid);

	//APS_LOG("tmd2772_get_ps_value  ps %d,", ps);
    /*Lenovo-sw zhuhc delete 2011-10-12 begin*/
	//return 1;
    /*Lenovo-sw zhuhc delete 2011-10-12 end*/

        mdelay(60);
	//tmd2772_read_ps(obj->client,temp_ps);

#if 0
if(1==Enable_ALSPS_LOG)
		{
        for(j = 0;j<cali_count;j++)
	printk("luosen data_test[%d]=%d,cali_num_end=%d\n",j,data_test[j],cali_num_end);
	printk("luosen calling_first=%d\n",calling_first);
	printk("luosen test_cali = %d, test_close = %d,test_far=%d\n",test_cali,test_close,test_far);		
	//printk("luosen close = %d,far_away=%d\n",ps_cali.close,ps_cali.far_away);
	}
			if((ps >ps_cali.close))
			{
				val = 0;  /*close*/
				val_temp = 0;
				intr_flag_value = 1;
			}
			else if((ps <ps_cali.far_away)&&(temp_ps[0] < ps_cali.far_away))
			{
				val = 1;  /*far away*/
				val_temp = 1;
				intr_flag_value = 0;
			}
			else
				val = val_temp;

			//APS_LOG("tmd2772_get_ps_value val  = %d",val);
#else
			//if((ps > atomic_read(&obj->ps_thd_val_high))&&(temp_ps[0]  > atomic_read(&obj->ps_thd_val_high)))
			if((ps > atomic_read(&obj->ps_thd_val_high)))
			{
				val = 0;  /*close*/
				val_temp = 0;
				intr_flag_value = 1;
			}
			//else if((ps < atomic_read(&obj->ps_thd_val_low))&&(temp_ps[0]  < atomic_read(&obj->ps_thd_val_low)))
			else if((ps < atomic_read(&obj->ps_thd_val_low)))
			{
				val = 1;  /*far away*/
				val_temp = 1;
				intr_flag_value = 0;
			}
			else
			       val = val_temp;	
#endif	
//end
	
	if(atomic_read(&obj->ps_suspend))
	{
		invalid = 1;
	}
	else if(1 == atomic_read(&obj->ps_deb_on))
	{
		unsigned long endt = atomic_read(&obj->ps_deb_end);
		if(time_after(jiffies, endt))
		{
			atomic_set(&obj->ps_deb_on, 0);
		}
		
		if (1 == atomic_read(&obj->ps_deb_on))
		{
			invalid = 1;
		}
	}
	else if (obj->als > 45000)
	{
		//invalid = 1;
		APS_DBG("ligh too high will result to failt proximiy\n");
		return 1;  /*far away*/
	}

	if(!invalid)
	{
		printk("ckt debug PS:  %05d => %05d\n", ps, val);
		return val;
	}	
	else
	{
		return -1;
	}	
}


/*----------------------------------------------------------------------------*/
/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
static void tmd2772_eint_work(struct work_struct *work)
{
	struct tmd2772_priv *obj = (struct tmd2772_priv *)container_of(work, struct tmd2772_priv, eint_work);
	int err;
	hwm_sensor_data sensor_data;
//	u8 buffer[1];
//	u8 reg_value[1];
	u8 databuf[2];
	int res = 0;
	APS_FUN();
	if((err = tmd2772_check_intr(obj->client)))
	{
		APS_ERR("tmd2772_eint_work check intrs: %d\n", err);
	}
	else
	{
		//get raw data
		tmd2772_read_ps(obj->client, &obj->ps);
		//mdelay(160);
		tmd2772_read_als_ch0(obj->client, &obj->als);
		APS_DBG("tmd2772_eint_work rawdata ps=%d als_ch0=%d!\n",obj->ps,obj->als);
		//printk("tmd2772_eint_work rawdata ps=%d als_ch0=%d!\n",obj->ps,obj->als);
		sensor_data.values[0] = tmd2772_get_ps_value(obj, obj->ps);
		sensor_data.value_divide = 1;
		sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;			
/*singal interrupt function add*/
#if 1
		if(intr_flag_value){
				//printk("yucong interrupt value ps will < 750");
				databuf[0] = TMD2772_CMM_INT_LOW_THD_LOW;	
				databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_low)) & 0x00FF);
				res = i2c_master_send(obj->client, databuf, 0x2);
				if(res <= 0)
				{
					return;
				}
				databuf[0] = TMD2772_CMM_INT_LOW_THD_HIGH;	
				databuf[1] = (u8)(((atomic_read(&obj->ps_thd_val_low)) & 0xFF00) >> 8);
				res = i2c_master_send(obj->client, databuf, 0x2);
				if(res <= 0)
				{
					return;
				}
				databuf[0] = TMD2772_CMM_INT_HIGH_THD_LOW;	
				databuf[1] = (u8)(0x00FF);
				res = i2c_master_send(obj->client, databuf, 0x2);
				if(res <= 0)
				{
					return;
				}
				databuf[0] = TMD2772_CMM_INT_HIGH_THD_HIGH; 
				databuf[1] = (u8)((0xFF00) >> 8);
				res = i2c_master_send(obj->client, databuf, 0x2);
				if(res <= 0)
				{
					return;
				}
		}
		else{	
				//printk("yucong interrupt value ps will > 900");
				databuf[0] = TMD2772_CMM_INT_LOW_THD_LOW;	
				databuf[1] = (u8)(0 & 0x00FF);
				res = i2c_master_send(obj->client, databuf, 0x2);
				if(res <= 0)
				{
					return;
				}
				databuf[0] = TMD2772_CMM_INT_LOW_THD_HIGH;	
				databuf[1] = (u8)((0 & 0xFF00) >> 8);
				res = i2c_master_send(obj->client, databuf, 0x2);
				if(res <= 0)
				{
					return;
				}
				databuf[0] = TMD2772_CMM_INT_HIGH_THD_LOW;	
				databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_high)) & 0x00FF);
				res = i2c_master_send(obj->client, databuf, 0x2);
				if(res <= 0)
				{
					return;
				}
				databuf[0] = TMD2772_CMM_INT_HIGH_THD_HIGH; 
				databuf[1] = (u8)(((atomic_read(&obj->ps_thd_val_high)) & 0xFF00) >> 8);;
				res = i2c_master_send(obj->client, databuf, 0x2);
				if(res <= 0)
				{
					return;
				}
		}
#endif
		//let up layer to know
		if((err = hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data)))
		{
		  APS_ERR("call hwmsen_get_interrupt_data fail = %d\n", err);
		}
	}
	tmd2772_clear_intr(obj->client);
	mt_eint_unmask(CUST_EINT_ALS_NUM);      
}


/****************************************************************************** 
 * Function Configuration
******************************************************************************/
static int tmd2772_open(struct inode *inode, struct file *file)
{
	file->private_data = tmd2772_i2c_client;

	if (!file->private_data)
	{
		APS_ERR("null pointer!!\n");
		return -EINVAL;
	}
	
	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int tmd2772_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

#if 0
static void tmd2772_WriteCalibration(struct PS_CALI_DATA_STRUCT *data_cali)
{

	   APS_LOG("tmd2772_WriteCalibration  1 %d," ,data_cali->close);
		   APS_LOG("tmd2772_WriteCalibration  2 %d," ,data_cali->far_away);
		   APS_LOG("tmd2772_WriteCalibration  3 %d,", data_cali->valid);
		   
	  if(data_cali->valid == 1)
	  {
	      if(data_cali->close < 100)
	      	{
		  	ps_cali.close = 200;
			ps_cali.far_away= 150;
			ps_cali.valid = 1;
	      	}
		  else if(data_cali->close > 900)
		  {
		  	ps_cali.close = 900;
			ps_cali.far_away= 750;
			ps_cali.valid = 1;
	      	}
		  else
		  {
			  ps_cali.close = data_cali->close;
			ps_cali.far_away= data_cali->far_away;
			ps_cali.valid = 1;
		  }
	  }
	  

}
#endif

#if 1
static int tmd2772_read_data_for_cali(struct i2c_client *client, struct PS_CALI_DATA_STRUCT *ps_data_cali)
{
     int i=0 ,err = 0,j = 0,m,n;
	 u16 data[13],sum,data_cali,temp,mid_data;
     int cali_num,cali_num_total;
        sum = 0;
	cali_num_total =0;	
	cali_num = 13;
	cali_num_end = 0;
	memset(&data[0],0,sizeof(u16)*cali_num);
	memset(&data_test[0],0,sizeof(u16)*cali_num);
	if(1==Enable_ALSPS_LOG)
	{
	for(j = 0;j<cali_num;j++)
	 	{
	printk("luosen 001 data_test[%d]=%d,data=%d\n",j,data_test[j],data[j]);
	       }
        } 
	     i=0;
	     j=0;
while(cali_num_total<=cali_num)
	{
		 		mdelay(60);//50
			if(err = tmd2772_read_ps(client,&data[j]))
			{
				printk("luosen tmd2772_read_data_for_cali fail: %d\n", i); 
				return 0;
			}
			mdelay(55);
			if((data[j]>0)&&data[j]<=900)
				{
				data_test[i]=data[j];
				i++;
				cali_num_total=i+1;
				}
			cali_num_end++;
			if(cali_num_end>30)
				break;
	}
	printk("luosenps i=%d,j=%d,cali_num_end=%d\n",i,j,cali_num_end);
#if 	0
   for(i = 0;i<cali_num;i++)
	 	{
	 		mdelay(60);//50
			if(err = tmd2772_read_ps(client,&data[i]))
			{
				printk("luosen tmd2772_read_data_for_cali fail: %d\n", i); 
				return 0;
			}
			data_test[i]=data[i];
	     #if 0		
			else
				{
			printk("luosen tmd2772_read_data_for_cali data[%d]=%d\n", i,data[i]); 
					sum += data[i];
					data_test[i]=data[i];
					cali_num_end=cali_num;
			}
		#endif
			mdelay(55);//160
	 	}
   #endif
  for(m=0;m<cali_num-1;m++)
  	{
  	for(n=0;n<cali_num-m-1;n++)
		{
		if(data_test[n]>data_test[n+1])
			{
			temp=data_test[n];
			data_test[n]=data_test[n+1];
			data_test[n+1]=temp;
			}
		}
  	}
mid_data=data_test[(cali_num+1)/2];
	 
  if(1==Enable_ALSPS_LOG)
  	{
     for(j = 0;j<cali_num;j++)
		{
	printk("luosen sort data[%d]=%d\n",j,data_test[j]);
	       }
     printk("luosen mid_data =%d\n",mid_data);
        }
	 
	 	//data_cali = sum/cali_num;
	 	data_cali = mid_data;
			ps_data_cali->close = data_cali + 100;
			ps_data_cali->far_away = data_cali + 60;
			test_cali=data_cali;
			test_close=ps_data_cali->close ;
			test_far=ps_data_cali->far_away;
			 if(ps_data_cali->close > 900)
	 	{
		  	ps_data_cali->close = 900;
			ps_data_cali->far_away = 750;
			err= 0;
			}
			//if(data_cali>600)
			//return -1;
		//	if(data_cali<=100)
           return 1;
}
#else
static int tmd2772_read_data_for_cali(struct i2c_client *client, struct PS_CALI_DATA_STRUCT *ps_data_cali)
			{
     int i=0 ,err = 0,j = 0;
	 u16 data[20],sum,data_cali;
     int cali_num = 20;
        sum = 0;
	memset(&data[0],0,sizeof(u16)*20);
	memset(&data_test[0],0,sizeof(u16)*20);
	if(1==Enable_ALSPS_LOG)
	{
	for(j = 0;j<20;j++)
		{
	printk("luosen 001 data_test[%d]=%d,data=%d\n",j,data_test[j],data[j]);
			}
        } 
   for(i = 0;i<20;i++)
			{
	 		mdelay(60);//50
			if(err = tmd2772_read_ps(client,&data[i]))
			{
				printk("luosen tmd2772_read_data_for_cali fail: %d\n", i); 
				return 0;
			}
			else
			{
			printk("luosen tmd2772_read_data_for_cali data[%d]=%d\n", i,data[i]); 
				if(0==data[i])
					{
					cali_num--;//��������Ϊ0�����Ե�����������ƽ��ֵ��С
					}
				else if((data[i]>900)&&0==i)
					{
					data[i]=0;
					cali_num--;
					}
				 else if(i>0)
				 	{
					 	if((abs(data[i]-data[i-1])>20)&&data[i-1]>0)
					 	{		
						data[i]=data[i-1];
					 	}
					}
	          			sum += data[i];
					data_test[i]=data[i];
					cali_num_end=cali_num;
			}
			mdelay(55);//160
	 	}
	 	data_cali = sum/cali_num;
			ps_data_cali->close = data_cali + 60;
			ps_data_cali->far_away = data_cali + 40;
			test_cali=data_cali;
			test_close=ps_data_cali->close ;
			test_far=ps_data_cali->far_away;
	#if 0		
			if(data_cali>800)
				{
	databuf[0] = TMD2772_CMM_OFFSET;    
	databuf[1] = 0x7F;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
			}
				}
	#endif
		        if(ps_data_cali->close > 900)
		       {
		  	ps_data_cali->close = 900;
			ps_data_cali->far_away = 750;
			err= 0;
	         	}
			else  if(ps_data_cali->close < 100)
           return 1;
}
#endif
static int tmd2772_init_client_factory(struct i2c_client *client)
			{
	struct tmd2772_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];
	int res = 0;
	printk("--@@line :%d,funct:%s\n",__LINE__,__FUNCTION__);
	databuf[0] = TMD2772_CMM_ENABLE;
	databuf[1] = 0x00;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
			}
	databuf[0] = TMD2772_CMM_ATIME;
	databuf[1] = 0xEE;//0xF6
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}

	databuf[0] = TMD2772_CMM_PTIME;
	databuf[1] = 0xFF;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
	databuf[0] = TMD2772_CMM_WTIME;
	databuf[1] = 0xEE;//0xFC,this is suggest by FAE
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
	
	databuf[0] = TMD2772_CMM_CONFIG;
	databuf[1] = 0x00;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	 	}
	databuf[0] = TMD2772_CMM_PPCOUNT;
	databuf[1] = TMD2772_CMM_PPCOUNT_VALUE;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
	databuf[0] = TMD2772_CMM_CONTROL;
	databuf[1] = TMD2772_CMM_CONTROL_VALUE;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
	}
     databuf[0] = TMD2772_CMM_OFFSET;
     databuf[1] = 0;
     res = i2c_master_send(client, databuf, 0x2);
     if(res <= 0)
     {
		goto EXIT_ERR;
		return TMD2772_ERR_I2C;
     }

	 return TMD2772_SUCCESS;
EXIT_ERR:
	APS_ERR("reinit dev: %d\n", res);
	return res;
}
static int store_status(unsigned int *flag)
	 	{
	u8 databuf[1];
	int res;
	if((*flag == 0)&&(0 != store_enable_register))
	{
		databuf[0] = TMD2772_CMM_ENABLE;
		databuf[1] = store_enable_register;
		res = i2c_master_send(tmd2772_i2c_client, databuf, 0x2);
		if(res <= 0)
		{
			return -1;
	 	}
	}
	if(*flag == 1)
	{
    	databuf[0] = TMD2772_CMM_ENABLE;
    	res = i2c_master_send(tmd2772_i2c_client, databuf, 0x1);
    	if(res <= 0)
    	{
    		return -1;
    	}
    	res = i2c_master_recv(tmd2772_i2c_client, databuf, 0x1);
    	if(res <= 0)
    	{
    		return -1;
    	}
    	store_enable_register = databuf[0];
	}
	return 0;
}


/*----------------------------------------------------------------------------*/
static long tmd2772_unlocked_ioctl(struct file *file, unsigned int cmd,
       unsigned long arg)
{
	struct i2c_client *client = (struct i2c_client*)file->private_data;
	struct tmd2772_priv *obj = i2c_get_clientdata(client);  
	long err = 0;
	void __user *ptr = (void __user*) arg;
	unsigned int dat = 0;
	uint32_t enable;
	//struct PS_CALI_DATA_STRUCT ps_cali_temp;
	uint32_t value=0;
	unsigned int enable_flag=0;

	switch (cmd)
	{
		case ALSPS_SET_PS_MODE:
			if(copy_from_user(&enable, ptr, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			if(enable)
			{
			#if OFFDATA_DEFAULT
				if(err = tmd2772_init_client_factory(client))
				{
					APS_ERR("init factory ps fail: %ld\n", err);
					goto err_out;
				}
				offset_data = 0;
			#endif
				if((err = tmd2772_enable_ps(obj->client, 1)))
				{
					APS_ERR("enable ps fail: %ld\n", err); 
					goto err_out;
				}
				
				set_bit(CMC_BIT_PS, &obj->enable);
			}
			else
			{
				if((err = tmd2772_enable_ps(obj->client, 0)))
				{
					APS_ERR("disable ps fail: %ld\n", err); 
					goto err_out;
				}
				
				clear_bit(CMC_BIT_PS, &obj->enable);
			}
			break;

		case ALSPS_GET_PS_MODE:
			enable = test_bit(CMC_BIT_PS, &obj->enable) ? (1) : (0);
			if(copy_to_user(ptr, &enable, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_GET_PS_DATA:    
			if((err = tmd2772_read_ps(obj->client, &obj->ps)))
			{
				goto err_out;
			}
			
			dat = tmd2772_get_ps_value(obj, obj->ps);
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}  
			break;

		case ALSPS_GET_PS_RAW_DATA:    
			if((err = tmd2772_read_ps(obj->client, &obj->ps)))
			{
				goto err_out;
			}
			
			dat = obj->ps;
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}  
			break;              

		case ALSPS_SET_ALS_MODE:
			if(copy_from_user(&enable, ptr, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			if(enable)
			{
				if((err = tmd2772_enable_als(obj->client, 1)))
				{
					APS_ERR("enable als fail: %ld\n", err); 
					goto err_out;
				}
				set_bit(CMC_BIT_ALS, &obj->enable);
			}
			else
			{
				if((err = tmd2772_enable_als(obj->client, 0)))
				{
					APS_ERR("disable als fail: %ld\n", err); 
					goto err_out;
				}
				clear_bit(CMC_BIT_ALS, &obj->enable);
			}
			break;

		case ALSPS_GET_ALS_MODE:
			enable = test_bit(CMC_BIT_ALS, &obj->enable) ? (1) : (0);
			if(copy_to_user(ptr, &enable, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_GET_ALS_DATA: 
			if((err = tmd2772_read_als(obj->client, &obj->als)))
			{
				goto err_out;
			}

			dat = tmd2772_get_als_value(obj, obj->als);
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}              
			break;

		case ALSPS_GET_ALS_RAW_DATA:    
			if((err = tmd2772_read_als(obj->client, &obj->als)))
			{
				goto err_out;
			}

			dat = obj->als;
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}              
			break;
			
              case ALSPS_GET_PS_CALI:
            mutex_lock(&mutex);
            tmd2772_ps_calibrate(tmd2772_obj->client);
            mutex_unlock(&mutex);
			if(copy_to_user(ptr, &offset_data, sizeof(offset_data)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;
			
		case ALSPS_RESET_PS:
			if(copy_from_user(&enable, ptr, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			if(enable)
			{
                mutex_lock(&mutex);
                offset_data = 0;
                tmd2772_ps_calibrate_call(tmd2772_obj->client);
                tmd2772_init_client(tmd2772_obj->client);
                if(err = tmd2772_enable_ps(tmd2772_obj->client, 1))
                {
                    mutex_unlock(&mutex);
                    goto err_out;
                }
                mutex_unlock(&mutex);
			}
			break;
			
			
		case ALSPS_SET_PS_CALI:
			if(copy_from_user(&dat, ptr, sizeof(dat)))
			{
				APS_LOG("copy_from_user\n");
				err = -EFAULT;
				break;	  
			}
			#if OFFDATA_DEFAULT
			   enable_flag = 1;
			   if((err = store_status(&enable_flag)))
					goto err_out;
			   offset_data = dat;
			   printk("ALSPS_SET_PS_CALI data:%d\n",offset_data);
			   // xiangfei.peng add 20140513 for update ps's threshold also,when entry alsps item without entry ps cali fisrt.
			   tmd2772_ps_calibrate_call(obj->client);
			   // xiangfei.peng add 20140513 for update ps's threshold also,when entry alsps item without entry ps cali fisrt.
			   tmd2772_init_client(client);
			   enable_flag = 0;
			   if((err = store_status(&enable_flag)))
					goto err_out;
			#endif
			break;
/*		case ALSPS_GET_PS_RAW_DATA_FOR_CALI:
			tmd2772_init_client_for_cali(obj->client);
			err = tmd2772_read_data_for_cali(obj->client,&ps_cali_temp);
			if(err)
			{
			   goto err_out;
			}
			tmd2772_init_client(obj->client);
			// tmd2772_enable_ps(obj->client, 1);
			tmd2772_enable(obj->client, 0);
			if(copy_to_user(ptr, &ps_cali_temp, sizeof(ps_cali_temp)))
			{
				err = -EFAULT;
				goto err_out;
			}              
			break;
*/
		default:
			APS_ERR("%s not supported = 0x%04x", __FUNCTION__, cmd);
			err = -ENOIOCTLCMD;
			break;
	}

	err_out:
	return err;    
}
/*----------------------------------------------------------------------------*/
static ssize_t tmd2772_show_als(struct device_driver *ddri, char *buf)
{
	int res;
	if(!tmd2772_obj)
	{
		APS_ERR("tmd2772_obj is null!!\n");
		return 0;
	}
	if((res = tmd2772_read_als_ch0(tmd2772_obj->client, &tmd2772_obj->als)))
	{
		return snprintf(buf, PAGE_SIZE, "ERROR: %d\n", res);
	}
	else
	{
		return snprintf(buf, PAGE_SIZE, "0x%04X\n", tmd2772_obj->als);     
	}
}
static ssize_t tmd2772_show_ps(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	if(!tmd2772_obj)
	{
		APS_ERR("tmd2772_obj is null!!\n");
		return 0;
	}
	if((res = tmd2772_read_ps(tmd2772_obj->client, &tmd2772_obj->ps)))
	{
		return snprintf(buf, PAGE_SIZE, "ERROR: %d\n", res);
	}
	else
	{
		return snprintf(buf, PAGE_SIZE, "ps_dec= %d\n", tmd2772_obj->ps);     
	}
}
static ssize_t tmd2772_show_config(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	if(!tmd2772_obj)
	{
		APS_ERR("tmd2772_obj is null!!\n");
		return 0;
	}
	res = snprintf(buf, PAGE_SIZE, "(%d %d %d %d %d %d %d %d %d %d)\n",
		atomic_read(&tmd2772_obj->i2c_retry), atomic_read(&tmd2772_obj->als_debounce), 
		atomic_read(&tmd2772_obj->ps_mask), atomic_read(&tmd2772_obj->ps_debounce),
	        atomic_read(&tmd2772_obj->ps_thd_val_high),atomic_read(&tmd2772_obj->ps_thd_val_low),
	     offset_data,ps_cali.valid,ps_cali.close,ps_cali.far_away);
	return res;    
}
static ssize_t tmd2772_store_ps(struct device_driver *ddri, const char *buf, size_t count)
{
	if(!tmd2772_obj)
	{
		APS_ERR("tmd2772_obj is null!!\n");
		return 0;
	}
	if(2 == sscanf(buf,"%d %d",&en_ps,&ps_value))
	{
		printk("--@en_ps:%d,ps_value:%d\n",en_ps,ps_value);
	}
	else
	{
		printk("-@tmd2772_store_ps is wrong!\n");
	}
	return count;
}
static ssize_t tmd2772_store_config(struct device_driver *ddri, const char *buf, size_t count)
{
	int retry, als_deb, ps_deb, mask, thres, thrh, thrl,valid,ps_close,ps_far_away,setdata;
	if(!tmd2772_obj)
	{
		APS_ERR("tmd2772_obj is null!!\n");
		return 0;
	}
	if(10 == sscanf(buf, "%d %d %d %d %d %d %d %d %d %d", &retry, &als_deb, &mask, &ps_deb,&thrh,&thrl,\
						&setdata,&valid,&ps_close,&ps_far_away))
	{ 
		atomic_set(&tmd2772_obj->i2c_retry, retry);
		atomic_set(&tmd2772_obj->als_debounce, als_deb);
		atomic_set(&tmd2772_obj->ps_mask, mask);
		atomic_set(&tmd2772_obj->ps_debounce, ps_deb);
		atomic_set(&tmd2772_obj->ps_thd_val_high, thrh);
		atomic_set(&tmd2772_obj->ps_thd_val_low, thrl);
		offset_data = setdata;
		ps_cali.valid = valid;
		ps_cali.close = ps_close;
		ps_cali.far_away = ps_far_away;
	}
	else
	{
		APS_ERR("invalid content: '%s', length = %d\n", buf, count);
	}
	return count;    
}
static ssize_t tmd2772_show_status(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;
	if(!tmd2772_obj)
	{
		APS_ERR("tmd2772_obj is null!!\n");
		return 0;
	}
	if(tmd2772_obj->hw)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST:i2c_num= %d\nppcount=%x\ncmm=%x\nhigh=%d\nlow=%d\n", 
			tmd2772_obj->hw->i2c_num, TMD2772_CMM_PPCOUNT_VALUE,  TMD2772_CMM_CONTROL_VALUE,
			tmd2772_obj->hw->ps_threshold_high,tmd2772_obj->hw->ps_threshold_low);
	}
	else
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: NULL\n");
	}
	len += snprintf(buf+len, PAGE_SIZE-len, "REGS: %02X %02X %02lX %02lX\n", 
				atomic_read(&tmd2772_obj->als_cmd_val), atomic_read(&tmd2772_obj->ps_cmd_val), 
				tmd2772_obj->enable, tmd2772_obj->pending_intr);
	len += snprintf(buf+len, PAGE_SIZE-len, "MISC: %d %d\n", atomic_read(&tmd2772_obj->als_suspend), atomic_read(&tmd2772_obj->ps_suspend));
	return len;
}

static int TMD2772_ReadChipInfo(struct i2c_client *client, char *buf, int bufsize)
{
	u8 databuf[10];    

	memset(databuf, 0, sizeof(u8)*10);

	if((NULL == buf)||(bufsize<=30))
	{
		return -1;
	}
	
	if(NULL == client)
	{
		*buf = 0;
		return -2;
	}

	sprintf(buf, "TMD2772 Chip");
	return 0;
}

static ssize_t show_chipinfo_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = tmd2772_i2c_client;
	char strbuf[256];
	if(NULL == client)
	{
		printk("i2c client is null!!\n");
		return 0;
	}
	
	TMD2772_ReadChipInfo(client, strbuf, 256);
	return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);        
}


static ssize_t tmd2772_show_pscalibrate(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	mutex_lock(&mutex);
	if(!tmd2772_obj)
	{
		APS_ERR("tmd2772_obj is null!!\n");
		mutex_unlock(&mutex);
		return 0;
	}
	tmd2772_ps_calibrate(tmd2772_obj->client);
	mutex_unlock(&mutex);
	return snprintf(buf, PAGE_SIZE, "%d\n",offset_data);
}


static DRIVER_ATTR(chipinfo,   S_IWUSR | S_IRUGO, show_chipinfo_value,      NULL);
static DRIVER_ATTR(als,     S_IWUSR | S_IRUGO, tmd2772_show_als,   NULL);
static DRIVER_ATTR(ps,      S_IWUSR | S_IRUGO, tmd2772_show_ps,    tmd2772_store_ps);
static DRIVER_ATTR(config,  S_IWUSR | S_IRUGO, tmd2772_show_config,tmd2772_store_config);
static DRIVER_ATTR(status,  S_IWUSR | S_IRUGO, tmd2772_show_status,  NULL);
static DRIVER_ATTR(pscalibrate,  S_IWUSR | S_IRUGO, tmd2772_show_pscalibrate,  NULL);
static struct driver_attribute *tmd2772_attr_list[] = {
    &driver_attr_chipinfo,
    &driver_attr_als,
    &driver_attr_ps,       
    &driver_attr_config,
    &driver_attr_status,
    &driver_attr_pscalibrate,
};
static int tmd2772_create_attr(struct device_driver *driver) 
{
	int idx, err = 0;
	int num = (int)(sizeof(tmd2772_attr_list)/sizeof(tmd2772_attr_list[0]));
	if (driver == NULL)
	{
		return -EINVAL;
	}
	for(idx = 0; idx < num; idx++)
	{
		if((err = driver_create_file(driver, tmd2772_attr_list[idx])))
		{            
			APS_ERR("driver_create_file (%s) = %d\n", tmd2772_attr_list[idx]->attr.name, err);
			break;
		}
	}    
	return err;
}
static int tmd2772_delete_attr(struct device_driver *driver)
{
	int idx ,err = 0;
	int num = (int)(sizeof(tmd2772_attr_list)/sizeof(tmd2772_attr_list[0]));
	if (!driver)
	return -EINVAL;
	for (idx = 0; idx < num; idx++) 
	{
		driver_remove_file(driver, tmd2772_attr_list[idx]);
	}
	return err;
}
static struct file_operations tmd2772_fops = {
	.owner = THIS_MODULE,
	.open = tmd2772_open,
	.release = tmd2772_release,
	.unlocked_ioctl = tmd2772_unlocked_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice tmd2772_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "als_ps",
	.fops = &tmd2772_fops,
};
/*----------------------------------------------------------------------------*/
static int tmd2772_i2c_suspend(struct i2c_client *client, pm_message_t msg) 
{
	//struct tmd2772_priv *obj = i2c_get_clientdata(client);    
	//int err;
	APS_FUN();    
#if 0
	if(msg.event == PM_EVENT_SUSPEND)
	{   
		if(!obj)
		{
			APS_ERR("null pointer!!\n");
			return -EINVAL;
		}
		
		atomic_set(&obj->als_suspend, 1);
		if(err = tmd2772_enable_als(client, 0))
		{
			APS_ERR("disable als: %d\n", err);
			return err;
		}

		atomic_set(&obj->ps_suspend, 1);
		if(err = tmd2772_enable_ps(client, 0))
		{
			APS_ERR("disable ps:  %d\n", err);
			return err;
		}
		
		tmd2772_power(obj->hw, 0);
	}
#endif
	return 0;
}
/*----------------------------------------------------------------------------*/
static int tmd2772_i2c_resume(struct i2c_client *client)
{
	//struct tmd2772_priv *obj = i2c_get_clientdata(client);        
	//int err;
	APS_FUN();
#if 0
	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return -EINVAL;
	}

	tmd2772_power(obj->hw, 1);
	if(err = tmd2772_init_client(client))
	{
		APS_ERR("initialize client fail!!\n");
		return err;        
	}
	atomic_set(&obj->als_suspend, 0);
	if(test_bit(CMC_BIT_ALS, &obj->enable))
	{
		if(err = tmd2772_enable_als(client, 1))
		{
			APS_ERR("enable als fail: %d\n", err);        
		}
	}
	atomic_set(&obj->ps_suspend, 0);
	if(test_bit(CMC_BIT_PS,  &obj->enable))
	{
		if(err = tmd2772_enable_ps(client, 1))
		{
			APS_ERR("enable ps fail: %d\n", err);                
		}
	}
#endif
	return 0;
}
/*----------------------------------------------------------------------------*/
static void tmd2772_early_suspend(struct early_suspend *h) 
{   /*early_suspend is only applied for ALS*/
	struct tmd2772_priv *obj = container_of(h, struct tmd2772_priv, early_drv);   
	int err;
	APS_FUN();    

	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return;
	}

	#if 0
	atomic_set(&obj->als_suspend, 1);
	if(test_bit(CMC_BIT_ALS, &obj->enable))
	{
		if((err = tmd2772_enable_als(obj->client, 0)))
		{
			APS_ERR("disable als fail: %d\n", err); 
		}
	}
	#endif
}
/*----------------------------------------------------------------------------*/
static void tmd2772_late_resume(struct early_suspend *h)
{   /*early_suspend is only applied for ALS*/
	struct tmd2772_priv *obj = container_of(h, struct tmd2772_priv, early_drv);         
	int err;
	APS_FUN();

	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return;
	}

        #if 0
	atomic_set(&obj->als_suspend, 0);
	if(test_bit(CMC_BIT_ALS, &obj->enable))
	{
		if((err = tmd2772_enable_als(obj->client, 1)))
		{
			printk("luosenalsps tmd2772  33 enable als fail: %d\n", err);        

		}
	}
	#endif
}

int tmd2772_ps_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	hwm_sensor_data* sensor_data;
	struct tmd2772_priv *obj = (struct tmd2772_priv *)self;
	
	APS_FUN(f);
	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			// Do nothing
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{				
				value = *(int *)buff_in;
				if(value)
				{
					#if DO_CALIBARTION
					tmd2772_ps_calibrate_call(obj->client);
					tmd2772_init_client(obj->client);
					#endif
					if((err = tmd2772_enable_ps(obj->client, 1)))
					{
						APS_ERR("enable ps fail: %d\n", err); 
						return -1;
					}
					set_bit(CMC_BIT_PS, &obj->enable);
					#if 0	
					if(err = tmd2772_enable_als(obj->client, 1))
					{
						APS_ERR("enable als fail: %d\n", err); 
						return -1;
					}
					set_bit(CMC_BIT_ALS, &obj->enable);
					#endif
				}
				else
				{
					if((err = tmd2772_enable_ps(obj->client, 0)))
					{
						APS_ERR("disable ps fail: %d\n", err); 
						return -1;
					}
					clear_bit(CMC_BIT_PS, &obj->enable);
					#if 0
					if(err = tmd2772_enable_als(obj->client, 0))
					{
						APS_ERR("disable als fail: %d\n", err); 
						return -1;
					}
					clear_bit(CMC_BIT_ALS, &obj->enable);
					#endif
				}
			}
			break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				APS_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				sensor_data = (hwm_sensor_data *)buff_out;	
				if(en_ps)
				{
				tmd2772_read_ps(obj->client, &obj->ps);
				
                                //mdelay(160);
				tmd2772_read_als_ch0(obj->client, &obj->als);
					//APS_ERR("tmd2772_ps_operate als data=%d!\n",obj->als);
				sensor_data->values[0] = tmd2772_get_ps_value(obj, obj->ps);
				}
				else
				{
					sensor_data->values[0] = ps_value;
				}
				sensor_data->value_divide = 1;
				sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;		
				//APS_LOG("tmd2772_ps_operate ps raw data=%d!, value=%d\n", obj->ps, sensor_data->values[0]);
			}
			break;
		default:
			APS_ERR("proxmy sensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}
	
	return err;
}

static int temp_als = 0;

int tmd2772_als_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	hwm_sensor_data* sensor_data;
	struct tmd2772_priv *obj = (struct tmd2772_priv *)self;

	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			// Do nothing
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;				
				if(value)
				{
					if((err = tmd2772_enable_als(obj->client, 1)))
					{
						APS_ERR("enable als fail: %d\n", err); 
						return -1;
					}
					set_bit(CMC_BIT_ALS, &obj->enable);
				}
				else
				{
					if((err = tmd2772_enable_als(obj->client, 0)))
					{
						APS_ERR("disable als fail: %d\n", err); 
						return -1;
					}
					clear_bit(CMC_BIT_ALS, &obj->enable);
				}
				
			}
			break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				APS_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				sensor_data = (hwm_sensor_data *)buff_out;
				/*yucong MTK add for fixing know issue*/
				tmd2772_read_als(obj->client, &obj->als);
				#if defined(MTK_AAL_SUPPORT)
				sensor_data->values[0] = obj->als;
				#else
				if(obj->als == 0)
				{
					sensor_data->values[0] = temp_als;				
				}else{
					u16 b[2];
					int i;
					for(i = 0;i < 2;i++){
					tmd2772_read_als(obj->client, &obj->als);
					b[i] = obj->als;
					}
					(b[1] > b[0])?(obj->als = b[0]):(obj->als = b[1]);
					sensor_data->values[0] = tmd2772_get_als_value(obj, obj->als);
					temp_als = sensor_data->values[0];
				}
				#endif
				sensor_data->value_divide = 1;
				sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
			}
			break;
		default:
			APS_ERR("light sensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}
	
	return err;
}


int tmd2772_read_mean_call(struct i2c_client *client , int n)
{
	struct tmd2772_priv *obj = i2c_get_clientdata(client);
	int prox_sum = 0, prox_mean = 0;
	int i, ret = 0;
	u16 prox_data[20];
	mdelay(10);
	for(i = 0; i < n; i++)
	{
		if(ret = tmd2772_read_ps(client, &prox_data[i]))
		{
			APS_ERR("tmd2772_read_data_for_cali fail: %d\n", i);
			return ret;
		}
		prox_sum += prox_data[i];
		mdelay(10);
	}
	prox_mean = prox_sum/n;
	return prox_mean;
}
static void tmd2772_ps_calibrate_call(struct i2c_client *client)
{
	struct tmd2772_priv *obj = i2c_get_clientdata(client);
	int prox_sum = 0, prox_mean = 0, prox_max = 0;
	int prox_threshold_hi = 0, prox_threshold_lo = 0;
	int i, ret = 0;
	u16 prox_data[20];
	u8 buffer[2];
	tmd2772_init_client_for_cali_call(obj->client);
	prox_mean = tmd2772_read_mean_call(client, 6);
	if(prox_mean < 800)
	{
             if(prox_mean < 500)
			 	{
			atomic_set(&obj->ps_thd_val_high, prox_mean+300);
			atomic_set(&obj->ps_thd_val_low, prox_mean+200);
		              }
			else
				{
			atomic_set(&obj->ps_thd_val_high, prox_mean+200);
			atomic_set(&obj->ps_thd_val_low, prox_mean+100);	
				}
		       printk("prox_mean<800 \n");
	}
	else
	{
		atomic_set(&obj->ps_thd_val_high, 800);
		atomic_set(&obj->ps_thd_val_low, 700);
		printk("prox_mean>800 \n");
	}
}
int tmd2772_read_mean(struct i2c_client *client , int n)
{
	struct tmd2772_priv *obj = i2c_get_clientdata(client);
	int prox_sum = 0, prox_mean = 0;
	int i, ret = 0;
	u16 prox_data[20];
	mdelay(10);
	for(i = 0; i < n; i++)
	{
		if(ret = tmd2772_read_ps(client, &prox_data[i]))
		{
			APS_ERR("tmd2772_read_data_for_cali fail: %d\n", i);
			return ret;
		}
		prox_sum += prox_data[i];
		mdelay(10);
	}
	prox_mean = prox_sum/n;
	printk("prox_mean %d \n", prox_mean);
	return prox_mean;
}
static void tmd2772_ps_calibrate(struct i2c_client *client)
{
	struct tmd2772_priv *obj = i2c_get_clientdata(client);
	int prox_sum = 0, prox_mean = 0, prox_max = 0;
	int prox_threshold_hi = 0, prox_threshold_lo = 0;
	int i, ret = 0;
	u16 prox_data[20];
	u8 buffer[2];
	int err;
	tmd2772_init_client_for_cali(obj->client);
	prox_mean = tmd2772_read_mean(client, 10);
	offset_data = 0;

	if((0 <=prox_mean)&&(prox_mean <50))//if prox_mean_clai is less than 200,plus prox_mean_clai
	{
		buffer[0] = TMD2772_CMM_OFFSET;
		offset_data = buffer[1] = 0x80 | 0x25;  // 0x80  not change. | 0x30  can change
		err= i2c_master_send(client, buffer, 0x2);
		if(err<= 0)
		{
			printk("prox_mean<50 error \n");
		}
		mdelay(5);//5ms
		prox_mean = tmd2772_read_mean(client, 10);
	}
	else if((50 <= prox_mean)&&(prox_mean< 120))//if prox_mean_clai is less than 200,plus prox_mean_clai
	{
		buffer[0] = TMD2772_CMM_OFFSET;
		offset_data = buffer[1] = 0x80 | 0x20;  // 0x80  not change.       | 0x30  can change
		err= i2c_master_send(client, buffer, 0x2);
		if(err<= 0)
		{
			printk("prox_mean<120 error \n");
		}
		mdelay(5);//5ms
		prox_mean = tmd2772_read_mean(client, 10);
	}
	else if((120 <= prox_mean)&&(prox_mean< 200))//if prox_mean_clai is less than 200,plus prox_mean_clai
	{
		buffer[0] = TMD2772_CMM_OFFSET;
		offset_data = buffer[1] = 0x80 | 0x15;  // 0x80  not change.       | 0x30  can change
		err= i2c_master_send(client, buffer, 0x2);
		if(err<= 0)
		{
			printk("prox_mean<200 error \n");
		}
		mdelay(5);//5ms
		prox_mean = tmd2772_read_mean(client, 10);
	}
	else if((600 <= prox_mean)&&(prox_mean < 800))
	{
		buffer[0] = TMD2772_CMM_OFFSET;
		offset_data = buffer[1] = 0x00 | 0x30;   // 0x00  not change.       | 0x30  can change
		err= i2c_master_send(client, buffer, 0x2);
		if(err<= 0)
		{
			printk("600 < prox_mean < 800 error \n");
		}
		mdelay(5);//5ms
		prox_mean = tmd2772_read_mean(client, 10);
	}
	else if((800 <= prox_mean)&&(prox_mean <= 1023))
	{
		buffer[0] = TMD2772_CMM_OFFSET;
		offset_data = buffer[1] = 0x00 | 0x7f;  // 0x80  not change.       | 0x30  can change
		err= i2c_master_send(client, buffer, 0x2);
		if(err<= 0)
		{
			printk("prox_mean<200 error \n");
		}
		mdelay(5);//5ms
		prox_mean = tmd2772_read_mean(client, 10);
	}
	else
	{
		offset_data = 0;
	}
	if(prox_mean > 800)
	{
		atomic_set(&obj->ps_thd_val_high, 800);
		atomic_set(&obj->ps_thd_val_low, 700);
		printk("prox_mean>900 \n");
	}
	else
	{
		atomic_set(&obj->ps_thd_val_high, prox_mean+120);
		atomic_set(&obj->ps_thd_val_low, prox_mean+80);
	}
}

/*----------------------------------------------------------------------------*/
static int tmd2772_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct tmd2772_priv *obj;
	struct hwmsen_object obj_ps, obj_als;
	int err = 0;

	if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto exit;
	}
	memset(obj, 0, sizeof(*obj));
	tmd2772_obj = obj;

	obj->hw = get_cust_alsps_hw();
	tmd2772_get_addr(obj->hw, &obj->addr);

	/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
	if(0 == obj->hw->polling_mode_ps)
	{
	INIT_WORK(&obj->eint_work, tmd2772_eint_work);
	}
	obj->client = client;
	i2c_set_clientdata(client, obj);	
	atomic_set(&obj->als_debounce, 50);
	atomic_set(&obj->als_deb_on, 0);
	atomic_set(&obj->als_deb_end, 0);
	atomic_set(&obj->ps_debounce, 10);
	atomic_set(&obj->ps_deb_on, 0);
	atomic_set(&obj->ps_deb_end, 0);
	atomic_set(&obj->ps_mask, 0);
	atomic_set(&obj->als_suspend, 0);
	atomic_set(&obj->als_cmd_val, 0xDF);
	atomic_set(&obj->ps_cmd_val,  0xC1);
	atomic_set(&obj->ps_thd_val_high,  obj->hw->ps_threshold_high);
	atomic_set(&obj->ps_thd_val_low,  obj->hw->ps_threshold_low);
	obj->enable = 0;
	obj->pending_intr = 0;
	obj->als_level_num = sizeof(obj->hw->als_level)/sizeof(obj->hw->als_level[0]);
	obj->als_value_num = sizeof(obj->hw->als_value)/sizeof(obj->hw->als_value[0]);  
	/*Lenovo-sw chenlj2 add 2011-06-03,modified gain 16 to 1/5 accoring to actual thing */
	obj->als_modulus = (400*100*TMD2772_ZOOM_TIME)/(1*150);//(1/Gain)*(400/Tine), this value is fix after init ATIME and CONTROL register value
										//(400)/16*2.72 here is amplify *100 //16
	BUG_ON(sizeof(obj->als_level) != sizeof(obj->hw->als_level));
	memcpy(obj->als_level, obj->hw->als_level, sizeof(obj->als_level));
	BUG_ON(sizeof(obj->als_value) != sizeof(obj->hw->als_value));
	memcpy(obj->als_value, obj->hw->als_value, sizeof(obj->als_value));
	atomic_set(&obj->i2c_retry, 3);
	set_bit(CMC_BIT_ALS, &obj->enable);
	set_bit(CMC_BIT_PS, &obj->enable);

	mutex_init(&mutex);
	tmd2772_i2c_client = client;
//add by sen.luo 
#if DO_CALIBARTION
#else
     calling_first = 1;
	tmd2772_init_client_for_cali(client);
	tmd2772_read_data_for_cali(client,&ps_cali);
#endif
	
	if((err = tmd2772_init_client(client)))
	{
		goto exit_init_failed;
	}
	APS_LOG("tmd2772_init_client() OK!\n");

	if((err = misc_register(&tmd2772_device)))
	{
		APS_ERR("tmd2772_device register failed\n");
		goto exit_misc_device_register_failed;
	}
/*
	if(err = tmd2772_create_attr(&tmd2772_alsps_driver.driver))
	{
		APS_ERR("create attribute err = %d\n", err);
		goto exit_create_attr_failed;
	}
*/
	if(err = tmd2772_create_attr(&(TMD2772_init_info.platform_diver_addr->driver)))
	{
		APS_ERR("create attribute err = %d\n", err);
		goto exit_create_attr_failed;
	}
	obj_ps.self = tmd2772_obj;
	/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
	if(1 == obj->hw->polling_mode_ps)
	//if(1)
	{
		obj_ps.polling = 1;
	}
	else
	{
		obj_ps.polling = 0;
	}

	obj_ps.sensor_operate = tmd2772_ps_operate;
	if((err = hwmsen_attach(ID_PROXIMITY, &obj_ps)))
	{
		APS_ERR("attach fail = %d\n", err);
		goto exit_create_attr_failed;
	}
	
	obj_als.self = tmd2772_obj;
	obj_als.polling = 1;
	obj_als.sensor_operate = tmd2772_als_operate;
	if((err = hwmsen_attach(ID_LIGHT, &obj_als)))
	{
		APS_ERR("attach fail = %d\n", err);
		goto exit_create_attr_failed;
	}


#if defined(CONFIG_HAS_EARLYSUSPEND)
	obj->early_drv.level    = EARLY_SUSPEND_LEVEL_STOP_DRAWING - 2,
	obj->early_drv.suspend  = tmd2772_early_suspend,
	obj->early_drv.resume   = tmd2772_late_resume,    
	register_early_suspend(&obj->early_drv);
#endif

	APS_LOG("%s: OK\n", __func__);
	TMD2772_init_flag = 0;
	return 0;

	exit_create_attr_failed:
	misc_deregister(&tmd2772_device);
	exit_misc_device_register_failed:
	exit_init_failed:
	//i2c_detach_client(client);
	//exit_kfree:
	kfree(obj);
	exit:
	tmd2772_i2c_client = NULL;           
//	MT6516_EINTIRQMask(CUST_EINT_ALS_NUM);  /*mask interrupt if fail*/
	APS_ERR("%s: err = %d\n", __func__, err);
	TMD2772_init_flag = -1;
	return err;
}
/*----------------------------------------------------------------------------*/
static int tmd2772_i2c_remove(struct i2c_client *client)
{
	int err;	
	/*
	if(err = tmd2772_delete_attr(&tmd2772_i2c_driver.driver))
	{
		APS_ERR("tmd2772_delete_attr fail: %d\n", err);
	} 
*/
	if(err = tmd2772_delete_attr(&(TMD2772_init_info.platform_diver_addr->driver)))
	{
		APS_ERR("tmd2772_delete_attr fail: %d\n", err);
	} 
	if((err = misc_deregister(&tmd2772_device)))
	{
		APS_ERR("misc_deregister fail: %d\n", err);    
	}
	
	tmd2772_i2c_client = NULL;
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));

	return 0;
}
/*----------------------------------------------------------------------------*/
static int tmd2772_probe(struct platform_device *pdev) 
{
	struct alsps_hw *hw = get_cust_alsps_hw();

	tmd2772_power(hw, 1);    
	//tmd2772_force[0] = hw->i2c_num;
	//tmd2772_force[1] = hw->i2c_addr[0];
	//APS_DBG("I2C = %d, addr =0x%x\n",tmd2772_force[0],tmd2772_force[1]);
	if(i2c_add_driver(&tmd2772_i2c_driver))
	{
		APS_ERR("add driver error\n");
		return -1;
	} 
	return 0;
}
/*----------------------------------------------------------------------------*/
static int tmd2772_remove(struct platform_device *pdev)
{
	struct alsps_hw *hw = get_cust_alsps_hw();
	APS_FUN();    
	tmd2772_power(hw, 0);    
	i2c_del_driver(&tmd2772_i2c_driver);
	return 0;
}
/*----------------------------------------------------------------------------*/
static struct platform_driver tmd2772_alsps_driver = {
	.probe      = tmd2772_probe,
	.remove     = tmd2772_remove,    
	.driver     = {
		.name  = "als_ps",
//		.owner = THIS_MODULE,
	}
};

static int TMD_remove(void)
{
    struct alsps_hw *hw = get_cust_alsps_hw();

    APS_FUN();    
    tmd2772_power(hw, 0);    
    i2c_del_driver(&tmd2772_i2c_driver);
    return 0;
}
/*----------------------------------------------------------------------------*/

static int TMD2772_local_init(void)
{
   struct alsps_hw *hw = get_cust_alsps_hw();
	APS_FUN();

	tmd2772_power(hw, 1);
	if(i2c_add_driver(&tmd2772_i2c_driver))
	{
		APS_ERR("add driver error\n");
		return -1;
	}
	if(-1 == TMD2772_init_flag)
	{
	   return -1;
	}
	
	return 0;
}
/*----------------------------------------------------------------------------*/
static int __init tmd2772_init(void)
{
	APS_FUN();
	struct alsps_hw *hw = get_cust_alsps_hw();
	APS_LOG("%s: i2c_number=%d\n", __func__,hw->i2c_num); 
	i2c_register_board_info(hw->i2c_num, &i2c_TMD2772, 1);
	#if 0
	if(platform_driver_register(&tmd2772_alsps_driver))
	{
		APS_ERR("failed to register driver");
		return -ENODEV;
	}
	#endif
	hwmsen_alsps_sensor_add(&TMD2772_init_info);
	return 0;
}
/*----------------------------------------------------------------------------*/
static void __exit tmd2772_exit(void)
{
	APS_FUN();
	//platform_driver_unregister(&tmd2772_alsps_driver);
}
/*----------------------------------------------------------------------------*/
module_init(tmd2772_init);
module_exit(tmd2772_exit);
/*----------------------------------------------------------------------------*/
MODULE_AUTHOR("Dexiang Liu");
MODULE_DESCRIPTION("tmd2772 driver");
MODULE_LICENSE("GPL");
