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

static int __devinit itop_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	drv_pr("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	return 0;
}

static int __devexit itop_i2c_remove(struct i2c_client *client)
{
	drv_pr("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	return 0;
}

static const struct i2c_device_id itop_i2c_id_table[] = {
	{ "at24c04", 0 },	
	{ "mpu6050", 0 },
	{}
};


/* 1. ∑÷≈‰/…Ë÷√i2c_driver */
static struct i2c_driver itop_i2c_driver = {
	.driver	= {
		.name	= "itop",
		.owner	= THIS_MODULE,
	},
	.probe		= itop_i2c_probe,
	.remove		= __devexit_p(itop_i2c_remove),
	.id_table	= itop_i2c_id_table,
};

static int __init itop_i2c_drv_init(void)
{
	/* 2. ◊¢≤·i2c_driver */
	i2c_add_driver(&itop_i2c_driver);
	
	return 0;
}

static void __exit itop_i2c_drv_exit(void)
{
	i2c_del_driver(&itop_i2c_driver);

	return ;
}

module_init(itop_i2c_drv_init);
module_exit(itop_i2c_drv_exit);

