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

static struct i2c_client *itop_i2c_client;

static const unsigned short addr_list[] = { 0x68, 0x50, I2C_CLIENT_END };

static int __init itop_i2c_dev_init(void)
{
	struct i2c_adapter *i2c_adap;
	struct i2c_board_info itop_i2c_info;

	memset(&itop_i2c_info, 0, sizeof(struct i2c_board_info));	
	strlcpy(itop_i2c_info.type, "at24c04", I2C_NAME_SIZE);

	i2c_adap = i2c_get_adapter(7);
	itop_i2c_client = i2c_new_probed_device(i2c_adap, &itop_i2c_info, addr_list, NULL);
	i2c_put_adapter(i2c_adap);

	if (itop_i2c_client)
		return 0;
	else
		return -ENODEV;
}

static void __exit itop_i2c_dev_exit(void)
{
	i2c_unregister_device(itop_i2c_client);

	return ;
}

module_init(itop_i2c_dev_init);
module_exit(itop_i2c_dev_exit);

