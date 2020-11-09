#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif

#define GPL2_CON		(0x11000100)
#define GPL2_DAT		(0x11000104)

#define GPK1_CON		(0x11000060)
#define GPK1_DAT		(0x11000064)

#define GPIOSIZE				3


/* 分配/设置/注册一个platform_device */

static struct resource led_resource[] = {
    [0] = {
        .start = GPL2_CON,
        .end   = GPL2_CON + GPIOSIZE,
        .flags = IORESOURCE_MEM,
    },
    
    [1] = {
        .start = GPL2_DAT,
        .end   = GPL2_DAT + GPIOSIZE,
        .flags = IORESOURCE_MEM,
    },

    [2] = {
        .start = GPK1_CON,
        .end   = GPK1_CON + GPIOSIZE,
        .flags = IORESOURCE_MEM,
    },
    
    [3] = {
        .start = GPK1_DAT,
        .end   = GPK1_DAT + GPIOSIZE,
        .flags = IORESOURCE_MEM,
    }

};


static void led_release(struct device * dev)
{
    return ;
}


static struct platform_device led_dev = {
    .name             = "myled",
    .id               = -1,
    .num_resources    = ARRAY_SIZE(led_resource),
    .resource         = led_resource,
    .dev              = { 
    	.release      = led_release, 
	},
};

static int __init led_dev_init(void)
{
	platform_device_register(&led_dev);
    drv_pr("%s : %s : %d\n",__FILE__, __func__, __LINE__);

    return 0;
}

static void __exit led_dev_exit(void)
{
	platform_device_unregister(&led_dev);

    return ;
}

module_init(led_dev_init);
module_exit(led_dev_exit);

