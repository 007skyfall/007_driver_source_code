/*********************************************************************************************
#####         上海嵌入式家园-开发板商城         #####
#####                    www.embedclub.com                        #####
#####             http://embedclub.taobao.com               #####

* File：	s3c2440_leds.c
* Author:	Hanson
* Desc：	Leds driver code
* History:	May 16th 2011
*********************************************************************************************/
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <mach/regs-clock.h>
#include <plat/regs-timer.h>
	 
#include <mach/regs-gpio.h>
// /home/student/linux-2.6.32.2/arch/arm/mach-s3c2410/include/mach/regs-gpio.h
#include <linux/cdev.h>

static int led_major = 0;     /* 主设备号 */
static struct cdev LedDevs;

/* 应用程序执行ioctl(fd, cmd, arg)时的第2个参数 */
#define LED_MAGIC 'k'
#define IOCTL_LED_ON _IOW (LED_MAGIC, 1, int)
#define IOCTL_LED_OFF _IOW (LED_MAGIC, 2, int)
#define IOCTL_LED_RUN _IOW (LED_MAGIC, 3, int)
#define IOCTL_LED_SHINE _IOW (LED_MAGIC, 4, int)
#define IOCTL_LED_ALLON _IOW (LED_MAGIC, 5, int)
#define IOCTL_LED_ALLOFF _IOW (LED_MAGIC, 6, int)



/* 用来指定LED所用的GPIO引脚 */
static unsigned long led_table [] = {
    S3C2410_GPB(5),
    S3C2410_GPB(6),
    S3C2410_GPB(7),
    S3C2410_GPB(8),
};


/* 应用程序对设备文件/dev/led执行open(...)时，
 * 就会调用s3c24xx_leds_open函数
 */
static int s3c2440_leds_open(struct inode *inode, struct file *file)
{
    int i;    
    for (i = 0; i < 4; i++) {
        // 设置GPIO引脚的功能：本驱动中LED所涉及的GPIO引脚设为输出功能
        s3c2410_gpio_cfgpin(led_table[i], S3C2410_GPIO_OUTPUT);
    }
    return 0;
}

//LEDS all light on
void leds_all_on()
{
    int i;
    for (i=0; i<4; i++) {
        s3c2410_gpio_setpin(led_table[i], 0);
    }
}

//LEDs all light off
void leds_all_off()
{
    int i;
    for (i=0; i<4; i++) {
        s3c2410_gpio_setpin(led_table[i], 1);
    }
}

/* 应用程序对设备文件/dev/leds执行ioctl(...)时，
 * 就会调用s3c24xx_leds_ioctl函数
 */
static int s3c2440_leds_ioctl(struct inode *inode, 
								struct file *file, 
								unsigned int cmd, 
								unsigned long arg)
{
    unsigned int data;
//    if (copy_from_user(&data, (unsigned int __user *)arg, sizeof(int)))
//        return -EFAULT;

// data = (unsigned int)arg;  // 方法一:数据传递

    if (__get_user(data, (unsigned int __user *)arg)) //方法二:指针参数传递
        return -EFAULT;

    switch(cmd) {
        case IOCTL_LED_ON:
            // 设置指定引脚的输出电平为0
            s3c2410_gpio_setpin(led_table[data], 0);
            return 0;

        case IOCTL_LED_OFF:
            // 设置指定引脚的输出电平为1
            s3c2410_gpio_setpin(led_table[data], 1);
            return 0;
            
        case IOCTL_LED_RUN:
            // 跑马灯
            {
               int i,j;
                leds_all_off();            
                //printk("IOCTL_LED_RUN");
                for (i=0;i<data;i++)
                    for (j=0;j<4;j++) {
                        s3c2410_gpio_setpin(led_table[j], 0);
                        mdelay(400); //delay 400ms
                        s3c2410_gpio_setpin(led_table[j], 1);
                        mdelay(400); //delay 400ms
                    }  
                return 0;
             }
          
        case IOCTL_LED_SHINE:
            // LED 闪烁
            {
                int i,j;
                leds_all_off();
                printk("IOCTL_LED_SHINE\n");
                for (i=0;i<data;i++) {
                    for (j=0;j<4;j++)
                        s3c2410_gpio_setpin(led_table[j], 0);
                    mdelay(400); //delay 400ms
                    for (j=0;j<4;j++)
                        s3c2410_gpio_setpin(led_table[j], 1);
                    mdelay(400);
                }
                return 0;
           }
        case IOCTL_LED_ALLON:
            // 设置指定引脚的输出电平为0
            leds_all_on();
            return 0;
        case IOCTL_LED_ALLOFF:
            // 设置指定引脚的输出电平为1
            leds_all_off();
            return 0;

        default:
            return -EINVAL;
    }
}

/* 这个结构是字符设备驱动程序的核心
 * 当应用程序操作设备文件时所调用的open、read、write等函数，
 * 最终会调用这个结构中指定的对应函数
 */
static struct file_operations s3c2440_leds_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   s3c2440_leds_open,     
    .ioctl  =   s3c2440_leds_ioctl,
};


/*
 * Set up the cdev structure for a device.
 */
static void led_setup_cdev(struct cdev *dev, int minor,
		struct file_operations *fops)
{
	int err, devno = MKDEV(led_major, minor);
    
	cdev_init(dev, fops);
	dev->owner = THIS_MODULE;
	dev->ops = fops;
	err = cdev_add (dev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk (KERN_NOTICE "Error %d adding Led%d", err, minor);
}

/*
 * 执行“insmod s3c24xx_leds.ko”命令时就会调用这个函数
 */

static int __init s3c2440_leds_init(void)
{
	int result;
	dev_t dev = MKDEV(led_major, 0);
	char dev_name[]="led";  /* 加载模式后，执行”cat /proc/devices”命令看到的设备名称 */

	/* Figure out our device number. */
	if (led_major)
		result = register_chrdev_region(dev, 1, dev_name);
	else {
		result = alloc_chrdev_region(&dev, 0, 1, dev_name);
		led_major = MAJOR(dev);
	}
	if (result < 0) {
		printk(KERN_WARNING "leds: unable to get major %d\n", led_major);
		return result;
	}
	if (led_major == 0)
		led_major = result;

	/* Now set up cdev. */
	led_setup_cdev(&LedDevs, 0, &s3c2440_leds_fops);
	printk("Led device installed, with major %d\n", led_major);
	printk("The device name is: %s\n", dev_name);
	return 0;
}

/*
 * 执行”rmmod s3c24xx_leds”命令时就会调用这个函数 
 */
static void __exit s3c2440_leds_exit(void)
{
    /* 卸载驱动程序 */
	cdev_del(&LedDevs);
	unregister_chrdev_region(MKDEV(led_major, 0), 1);
	printk("Led device uninstalled\n");
}

/* 这两行指定驱动程序的初始化函数和卸载函数 */
module_init(s3c2440_leds_init);
module_exit(s3c2440_leds_exit);

/* 描述驱动程序的一些信息，不是必须的 */
MODULE_AUTHOR("http://embedclub.taobao.com");             // 驱动程序的作者
MODULE_DESCRIPTION("s3c2440 LED Driver");   // 一些描述信息
MODULE_LICENSE("Dual BSD/GPL");                             // 遵循的协议

