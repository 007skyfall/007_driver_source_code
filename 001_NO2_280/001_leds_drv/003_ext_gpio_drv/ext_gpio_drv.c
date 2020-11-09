#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/device.h>


MODULE_LICENSE("GPL");

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif


#define GPM4_CON		(0x110002E4)
#define GPIOSIZE				8

static struct class 	*ext_gpio_drv_class  = NULL;
static struct device	*ext_gpio_drv_device = NULL;

volatile unsigned long *gpm4con = NULL;
volatile unsigned long *gpm4dat = NULL;

int major;

static int ext_gpio_drv_open(struct inode *inode, struct file *file)
{
	drv_pr("------------leds_drv_open------------\n");
	/* 配置GPM4_6为输出 */
	*gpm4con &= ~((0xf<<(4*6)))|(0x01<<(4*6));

	return 0;
}

static ssize_t ext_gpio_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val = 0;

	drv_pr("------------leds_drv_write------------\n");
	
	copy_from_user(&val, buf, count); //	copy_to_user();

	if (val == 1)
	{
		// HIGH
		*gpm4dat |= (0x01<<6);

	}
	else
	{
		// LOW
		*gpm4dat &= ~(0x01<<6);

	}
	
	return count;
}

static int exit_gpio_drv_close (struct inode *inode, struct file *file)
{
    // LOW
    *gpm4dat &= ~(0x01<<6);
    
    return 0;
}

static struct file_operations ext_gpio_drv_fops = {
    .owner    =     THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open     =     ext_gpio_drv_open,     
	.write	  = 	ext_gpio_drv_write,	
	.release  =     exit_gpio_drv_close,
};

static int __init ext_gpio_drv_init(void)
{
	int minor = 0;
    drv_pr("%s : %s : %d\n",__FILE__, __func__, __LINE__);
    
	//参数1为0则内核自动给分配设备号，返回值即为设备号
	major = register_chrdev(0, "ext_gpio_drv", &ext_gpio_drv_fops); // 注册, 告诉内核

	ext_gpio_drv_class  = class_create(THIS_MODULE, "ext_gpio_drv");
	ext_gpio_drv_device = device_create(ext_gpio_drv_class, NULL, MKDEV(major, minor), NULL, "ext_gpio_drv"); 
	
	gpm4con = (volatile unsigned long *)ioremap(GPM4_CON, GPIOSIZE);
	gpm4dat = gpm4con + 1;

	return 0;
}

static void __exit ext_gpio_drv_exit(void)
{
	unregister_chrdev(major, "ext_gpio_drv"); // 卸载

	device_destroy(ext_gpio_drv_class, MKDEV(major, 0));
	class_destroy(ext_gpio_drv_class);
	
	iounmap(gpm4con);

	return ;
}

module_init(ext_gpio_drv_init);
module_exit(ext_gpio_drv_exit);

