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

static int __devinit itop_i2c_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
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

static int itop_i2c_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	/* 能运行到这里, 表示该addr的设备是存在的
	 * 但是有些设备单凭地址无法分辨(A芯片的地址是0x50, B芯片的地址也是0x50)
	 * 还需要进一步读写I2C设备来分辨是哪款芯片
	 * detect就是用来进一步分辨这个芯片是哪一款，并且设置info->type
	 */
	
	printk("itop_i2c_detect : addr = 0x%x\n", client->addr);

	/* 进一步判断是哪一款 */
	
	strlcpy(info->type, "at24c04", I2C_NAME_SIZE);
	
	return 0;
}

static const unsigned short addr_list[] = { 0x68, 0x50, I2C_CLIENT_END };

/* 1. 分配/设置i2c_driver */
static struct i2c_driver itop_i2c_driver = {
	.class  = I2C_CLASS_HWMON, /* 表示去哪些适配器上找设备 */
	.driver	= {
		.name	= "itop",
		.owner	= THIS_MODULE,
	},
	.probe		= itop_i2c_probe,
	.remove		= __devexit_p(itop_i2c_remove),
	.id_table	= itop_i2c_id_table,
	.detect     = itop_i2c_detect,  /* 用这个函数来检测设备确实存在 */
	.address_list	= addr_list,   /* 这些设备的地址 */
};

static int __init itop_i2c_drv_init(void)
{
	/* 2. 注册i2c_driver */
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

