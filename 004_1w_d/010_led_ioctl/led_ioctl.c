#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include "myioctl.h"

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif

#define GPL2_CON		(0x11000100)
#define GPK1_CON		(0x11000060)
#define GPIOSIZE				4

volatile unsigned long *gpl2con = NULL;
volatile unsigned long *gpl2dat = NULL;

volatile unsigned long *gpk1con = NULL;
volatile unsigned long *gpk1dat = NULL;

#define NAME 		"led_ioctl"
int major	= 0;
char kbuf[128] = { 0 };

static ssize_t led_ioctl_read(struct file *file, char __user *ubuf, size_t size, loff_t *offs)
{
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;
}

static ssize_t led_ioctl_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offs)
{
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;
}


static int led_ioctl_open(struct inode *inode, struct file *file)
{
	drv_pr("------------itop_leds_drv_open------------\n");
	
	/* 配置GPL2_0为输出 */
	*gpl2con &= ~((0xf<<(4*0)))|(0x01<<(4*0));

	/* 配置GPK1_1为输出 */
	*gpk1con |= ~((0xf<<(4*1)))|(0x01<<(4*1));

	return 0;
}

static long led_ioctl_ioctl(struct file *file, unsigned int cmd, unsigned long args)
{
	drv_pr("------------itop_leds_drv_open------------\n");
	
	switch(cmd){
		case LED2_OP:{
				if(args){
					// LED2点灯
					*gpl2dat |= (0x01<<0);
					
				}else{
					// LED2熄灭
					*gpl2dat &= ~(0x01<<0);
				}
			}break;
		
		case LED3_OP:{
				if(args){
					
					// LED3点灯
					*gpk1dat |= (0x01<<1);
				}else{
					// LED3熄灭
					*gpk1dat &= ~(0x01<<1);
				}
			}break;
	}
	return 0;
}

static int led_ioctl_close(struct inode *inode, struct file *file)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;
}


static const struct file_operations fops = {
	.open 			= led_ioctl_open,
	.read 			= led_ioctl_read,
	.write 			= led_ioctl_write,
	.unlocked_ioctl = led_ioctl_ioctl,
	.release 		= led_ioctl_close,
};

static int __init led_ioctl_init(void)
{
	major = register_chrdev(0,NAME,&fops);
	if(major <= 0){
		printk("register chrdev error\n");
		return -EAGAIN;
	}
	gpl2con = ioremap(GPL2_CON,GPIOSIZE);
	if(gpl2con == NULL){
		printk("ioremap led2 con error\n");
		return -EAGAIN;
	}
	gpl2dat = gpl2con + 1;
	
	gpk1con = ioremap(GPK1_CON,GPIOSIZE);
	if(gpk1con == NULL){
		printk("ioremap led3 con error\n");
		return -EAGAIN;
	}
	gpk1dat = gpk1con + 1;
	
	return 0;
}
static void __exit led_ioctl_exit(void)
{
	iounmap(gpl2dat);
	iounmap(gpk1con);
	unregister_chrdev(major,NAME);

	return ;
}

module_init(led_ioctl_init);
module_exit(led_ioctl_exit);

MODULE_LICENSE("GPL");

