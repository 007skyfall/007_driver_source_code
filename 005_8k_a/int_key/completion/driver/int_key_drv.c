/*********************************************************************************************
#####         上海嵌入式家园-开发板商城         #####
#####                    www.embedclub.com                        #####
#####             http://embedclub.taobao.com               #####
* File：		int_key_drv.c
* Author:		Hanson
* Desc：	int key scan device driver 
* History:	May 16th 2011
*********************************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/gpio.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>

#define USING_TASKLET

static int key_major = 0;

struct key_irq_desc {
    	unsigned int irq;
	int pin;
	int pin_setting;
	int number;
    	char *name;	
};

/* 用来指定按键所用的外部中断引脚及中断触发方式, 名字 */
static struct key_irq_desc key_irqs [] = {
    {IRQ_EINT8, S3C2410_GPG(0), S3C2410_GPG0_EINT8, 0, "KEY1"}, /* K1 */
    {IRQ_EINT11, S3C2410_GPG(3),  S3C2410_GPG3_EINT11,  1, "KEY2"}, /* K2 */
    {IRQ_EINT13,  S3C2410_GPG(5),  S3C2410_GPG5_EINT13,   2, "KEY3"}, /* K3 */
    {IRQ_EINT14,  S3C2410_GPG(6),  S3C2410_GPG6_EINT14,   3, "KEY4"}, /* K3 */
    {IRQ_EINT15,  S3C2410_GPG(7),  S3C2410_GPG7_EINT15,   4, "KEY5"}, /* K3 */
    {IRQ_EINT19,  S3C2410_GPG(11),  S3C2410_GPG11_EINT19,  5, "KEY6"}, /* K4 */
};

/* 按键被按下的次数(准确地说，是发生中断的次数) */
static volatile int key_values[] = {0, 0, 0, 0, 0, 0};

/* 等待队列: 
 * 当没有按键被按下时，如果有进程调用key_read函数，
 * 它将休眠
 */
static DECLARE_WAIT_QUEUE_HEAD(key_waitq); //初始化一个等待队列头key_waitq

/* 中断事件标志, 中断服务程序将它置1，key_read将它清0 */
static volatile int ev_press = 0;
#ifdef USING_TASKLET

static struct tasklet_struct key_tasklet;
static void key_do_tasklet(unsigned long);
//DECLARE_TASKLET(key_tasklet, key_do_tasklet, 0);

static void key_do_tasklet(unsigned long data)
{
	printk("key_do_tasklet\n");
}
#endif

static irqreturn_t key_interrupt(int irq, void *dev_id)
{
	struct key_irq_desc *key_irqs = (struct key_irq_desc *)dev_id;
    	int up = s3c2410_gpio_getpin(key_irqs->pin);//arch/arm/plat-s3c24xx/gpio.c

	printk("<1>up=%d\n",up);
	if (up)
		key_values[key_irqs->number] = (key_irqs->number + 1) + 0x80;
	else
		key_values[key_irqs->number] += 1;
	
    	ev_press = 1;                  /* 表示中断发生了 */
    	wake_up_interruptible(&key_waitq);   /* 唤醒休眠的进程 */
	
#ifdef USING_TASKLET
	tasklet_schedule(&key_tasklet);
#endif
    
    	return IRQ_RETVAL(IRQ_HANDLED);
}


/* 应用程序对设备文件/dev/key执行open(...)时，
 * 就会调用key_open函数
 */
static int key_open(struct inode *inode, struct file *file)
{
    int i;
    int err;
    
    for (i = 0; i < sizeof(key_irqs)/sizeof(key_irqs[0]); i++) {
        // 注册中断处理函数
	s3c2410_gpio_cfgpin(key_irqs[i].pin,key_irqs[i].pin_setting);
        err = request_irq(key_irqs[i].irq, key_interrupt, 0, 
                          key_irqs[i].name, (void *)&key_irqs[i]);
	set_irq_type(key_irqs[i].irq, IRQ_TYPE_EDGE_FALLING);//<linux/irq.h> 
	// IRQ_TYPE_EDGE_RISING, IRQ_TYPE_EDGE_BOTH, IRQ_TYPE_EDGE_HIGH, 
	// IRQ_TYPE_EDGE_LOW
        if (err)
            break;
    }

    if (err) {
        // 释放已经注册的中断
        i--;
        for (; i >= 0; i--) {
		disable_irq(key_irqs[i].irq);
            	free_irq(key_irqs[i].irq, (void *)&key_irqs[i]);
        }
        return -EBUSY;
    }
    
    return 0;
}


/* 应用程序对设备文件/dev/key执行close(...)时，
 * 就会调用key_close函数
 */
static int key_close(struct inode *inode, struct file *file)
{
    int i;
    
    for (i = 0; i < sizeof(key_irqs)/sizeof(key_irqs[0]); i++) {
        // 释放已经注册的中断
	disable_irq(key_irqs[i].irq);
        free_irq(key_irqs[i].irq, (void *)&key_irqs[i]);
    }

    return 0;
}


/* 应用程序对设备文件/dev/key执行read(...)时，
 * 就会调用key_read函数
 */
static int key_read(struct file *filp, char __user *buff, 
                        size_t count, loff_t *offp)
{
    unsigned long err;

	if (!ev_press) {
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		else
			/* 如果ev_press等于0，休眠,直到key_waitq被唤醒,并且ev_press为真*/
			wait_event_interruptible(key_waitq, ev_press);
	}
    
    /* 执行到这里时，ev_press等于1，将它清0 */
    ev_press = 0;

    /* 将按键状态复制给用户，并清0 */
    err = copy_to_user(buff, (const void *)key_values, min(sizeof(key_values), count));
    memset((void *)key_values, 0, sizeof(key_values));

    return err ? -EFAULT : min(sizeof(key_values), count);
}

/**************************************************
* 当用户程序调用select函数时，本函数被调用
* 如果有按键数据，则select函数会立刻返回
* 如果没有按键数据，本函数使用poll_wait等待
**************************************************/
static unsigned int key_poll(struct file *file,
        			 struct poll_table_struct *wait)
{
	unsigned int mask = 0;
    	poll_wait(file, &key_waitq, wait);
    	if (ev_press)
        	mask |= POLLIN | POLLRDNORM;
    	return mask;
}


/* 这个结构是字符设备驱动程序的核心
 * 当应用程序操作设备文件时所调用的open、read、write等函数，
 * 最终会调用这个结构中的对应函数
 */
static struct file_operations key_fops = {
    .owner   =   THIS_MODULE,    /* 这是一个宏，指向编译模块时自动创建的__this_module变量 */
    .open    =   key_open,
    .release =   key_close, 
    .read    =   key_read,
    .poll    =   key_poll,
};


/*
 * Set up the cdev structure for a device.
 */
static void key_setup_cdev(struct cdev *dev, int minor,
                struct file_operations *fops)
{
        int err, devno = MKDEV(key_major, minor);

        cdev_init(dev, fops);
        dev->owner = THIS_MODULE;
        dev->ops = fops;
        err = cdev_add (dev, devno, 1);
        /* Fail gracefully if need be */
        if (err)
                printk (KERN_NOTICE "Error %d adding key%d", err, minor);
}


/*
 * We export one key device.  There's no need for us to maintain any
 * special housekeeping info, so we just deal with raw cdev.
 */
static struct cdev key_cdev;


/*
 * 执行"insmod key_drv.ko" 命令时就会调用这个函数
 */
static int __init userkey_init(void)
//static int key_init(void)
{
        int result;
        dev_t dev = MKDEV(key_major, 0);
	char dev_name[]="key";
                                                                                                         
        /* Figure out our device number. */
        if (key_major)
                result = register_chrdev_region(dev, 1, dev_name);
        else {
                result = alloc_chrdev_region(&dev, 0, 1, dev_name);
                key_major = MAJOR(dev);
        }
        if (result < 0) {
                printk(KERN_WARNING "key: unable to get major %d\n", key_major);
                return result;
        }
        if (key_major == 0)
                key_major = result;

#ifdef USING_TASKLET
	tasklet_init(&key_tasklet, key_do_tasklet, 0);
#endif                                                                                                         
        /* Now set up cdev. */
        key_setup_cdev(&key_cdev, 0, &key_fops);
        printk("key device installed, with major %d\n", key_major);
	printk("The device name is: /dev/%s\n", dev_name);
 
    return 0;
}

/*
 * 执行rmod key_drv”命令时就会调用这个函数 
 */
static void __exit userkey_exit(void)
//static void key_exit(void)
{
        cdev_del(&key_cdev);
        unregister_chrdev_region(MKDEV(key_major, 0), 1);
#ifdef USING_TASKLET
	tasklet_kill(&key_tasklet);
#endif
        printk("key device uninstalled\n");
}

/* 这两行指定驱动程序的初始化函数和卸载函数 */
module_init(userkey_init);
module_exit(userkey_exit);

/* 描述驱动程序的一些信息，不是必须的 */
MODULE_AUTHOR("Hanson He");             // 驱动程序的作者
MODULE_DESCRIPTION("S3C2410/S3C2440 KEY Driver");   // 一些描述信息
MODULE_LICENSE("Dual BSD/GPL");                              // 遵循的协议

