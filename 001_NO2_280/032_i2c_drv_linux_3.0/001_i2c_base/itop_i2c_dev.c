#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif

static struct i2c_board_info itop_i2c_drv_info = {	
	I2C_BOARD_INFO("at24c04", 0x50),		
	I2C_BOARD_INFO("mpu6050", 0x68),
};

static struct i2c_client *itop_i2c_client;

static int __init itop_i2c_dev_init(void)
{
	struct i2c_adapter *i2c_adap;

	i2c_adap = i2c_get_adapter(7);
	itop_i2c_client = i2c_new_device(i2c_adap, &itop_i2c_drv_info);
	i2c_put_adapter(i2c_adap);
	
	return 0;
}

static void __exit top_i2c_dev_exit(void)
{
	i2c_unregister_device(itop_i2c_client);

	return ;

}

module_init(itop_i2c_dev_init);
module_exit(top_i2c_dev_exit);

