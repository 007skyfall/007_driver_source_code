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

#define GPL2_CON		(0x11000100)
#define GPK1_CON		(0x11000060)
#define GPIOSIZE				8

static struct class 	*itop_leds_drv_class;
static struct device	*itop_leds_drv_device;

volatile unsigned long *gpl2con = NULL;
volatile unsigned long *gpl2dat = NULL;

volatile unsigned long *gpk1con = NULL;
volatile unsigned long *gpk1dat = NULL;

int major;

static int itop_leds_drv_open(struct inode *inode, struct file *file)
{
	drv_pr("------------itop_leds_drv_open------------\n");
    
	/* 配置GPL2_0为输出 */
	*gpl2con &= ~((0xf<<(4*0)))|(0x01<<(4*0));

	/* 配置GPK1_1为输出 */
	*gpk1con |= ~((0xf<<(4*1)))|(0x01<<(4*1));

    return 0;
}

static ssize_t itop_leds_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val = 0;

	drv_pr("------------itop_leds_drv_write------------\n");
	
#if 1
//    unsigned long copy_from_user(void *to, const void __user *from, unsigned long n)
	copy_from_user(&val, buf, count); //	copy_to_user();

	if (val == 1)
	{
		// LED2点灯
		*gpl2dat |= (0x01<<0);

		// LED3点灯
		*gpk1dat |= (0x01<<1);

	}
	else
	{
		// LED2熄灭
		*gpl2dat &= ~(0x01<<0);

		// LED3熄灭
		*gpk1dat &= ~(0x01<<1);
	}

#endif
	
	return count;
}

static int itop_leds_drv_close(struct inode *inode, struct file *file)
{
    // LED2熄灭
    *gpl2dat &= ~(0x01<<0);

    // LED3熄灭
    *gpk1dat &= ~(0x01<<1);
    
    return 0;
}


static struct file_operations itop_leds_drv_fops = {
    .owner   =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open    =   itop_leds_drv_open,     
	.write	 =	 itop_leds_drv_write,
	.release =   itop_leds_drv_close,
};

static int __init itop_leds_drv_init(void)
{
    drv_pr("%s : %s : %d\n",__FILE__, __func__, __LINE__);
    
	int minor = 0;
	//参数1为0则内核自动给分配设备号，返回值即为设备号
	major = register_chrdev(0, "itop_leds_drv", &itop_leds_drv_fops); // 注册, 告诉内核

    //自动创建设备节点
	itop_leds_drv_class = class_create(THIS_MODULE, "ledsdrv");
	for(minor = 2; minor < 4; minor++){
	/* /dev/led2  /dev/led3 */
	itop_leds_drv_device = device_create(itop_leds_drv_class, NULL, MKDEV(major, minor), NULL, "led%d",minor); 
	}
	
	gpl2con = (volatile unsigned long *)ioremap(GPL2_CON, GPIOSIZE);
	gpl2dat = gpl2con + 1;

//将LED2默认为熄灭状态
#if 1
	*gpl2con &= ~((0xf<<(4*0)))|(0x01<<(4*0));//设置为输出
	*gpl2dat &= ~(0x01<<0);//输出低电平
#endif

	gpk1con = (volatile unsigned long *)ioremap(GPK1_CON, GPIOSIZE);
	gpk1dat = gpk1con + 1;

	return 0;
}

static void __exit itop_leds_drv_exit(void)
{
    drv_pr("%s : %s : %d\n",__FILE__, __func__, __LINE__);
	
	unregister_chrdev(major, "itop_leds_drv"); // 卸载

	device_destroy(itop_leds_drv_class, MKDEV(major, 0));
	class_destroy(itop_leds_drv_class);
	
	iounmap(gpl2con);
	iounmap(gpk1con);

	return ;
}

module_init(itop_leds_drv_init);
module_exit(itop_leds_drv_exit);

