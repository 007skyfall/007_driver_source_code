#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/platform_device.h>


MODULE_LICENSE("GPL");

#define  GPL2CON  0x11000100
#define  GPL2DAT  0x11000104

struct resource res[2] = {
	[0] = {
		.start = GPL2CON,
		.end = GPL2CON+3,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = GPL2DAT,
		.end = GPL2DAT+3,
		.flags = IORESOURCE_MEM,
	},
};

static void demo_release(struct device *dev)
{
	printk("%s,%d\n", __func__, __LINE__);

	return ;
}

static struct platform_device  pdevice = {
	.name = "platform_led",
	.num_resources = ARRAY_SIZE(res),
	.resource = res,
	.dev = {
		.release = demo_release,
	},
};

static int __init demo_init(void)
{
	printk("%s,%d\n", __func__, __LINE__);

	platform_device_register(&pdevice);

	return 0;
}

static void __exit demo_exit(void)
{
	printk("%s,%d\n", __func__, __LINE__);
	
	platform_device_unregister(&pdevice);
	
	return ;
}

module_init(demo_init);
module_exit(demo_exit);
