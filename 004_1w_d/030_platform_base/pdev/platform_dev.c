#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif

struct resource res[] = {
	[0] = {
		.start = 0x114001e0,
		.end   = 0x114001e0 + 50,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = 100,
		.end   = 100,
		.flags = IORESOURCE_IRQ,
	},
};

static void platform_dev_release(struct device *dev)
{
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);
}

static struct platform_device platform_dev = {
	.name = "hello",
	.id   = -1,
	.dev  ={
		.release = platform_dev_release,
	}, 
	.num_resources = ARRAY_SIZE(res),
	.resource = res,
};

static int __init platform_dev_init(void)
{
	return platform_device_register(&platform_dev);
	
	
}
static void __exit platform_dev_exit(void)
{
	platform_device_unregister(&platform_dev);
}

module_init(platform_dev_init);
module_exit(platform_dev_exit);

