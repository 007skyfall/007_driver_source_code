/*********************************************************************************************
#####         上海嵌入式家园-开发板商城         #####
#####                    www.embedclub.com                        #####
#####             http://embedclub.taobao.com               #####

* File：	nosem.c
* Author:	Hanson
* Desc：	simple char device driver, no semaphore
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
#include <linux/cdev.h>

static int test_major = 0;

MODULE_AUTHOR("Hanson He");
MODULE_LICENSE("Dual BSD/GPL");

#define GLOBALMEM_SIZE	0x1000	/*全局内存最大4K字节*/
unsigned char mem[GLOBALMEM_SIZE]; /*全局内存*/


/*
 * Open the device; in fact, there's nothing to do here.
 */
int test_open (struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t test_read(struct file *file, char __user *buff, size_t count, loff_t *offp)
{
	return 0;
}

ssize_t test_write(struct file *file, const char __user *buff, size_t count, loff_t *offp)
{
	int i;

	unsigned long p =  *offp;
	int ret = 0;

	/*分析和获取有效的写长度*/
	if (p >= GLOBALMEM_SIZE)
		return count ?  - ENXIO: 0;
	if (count > GLOBALMEM_SIZE - p)
		count = GLOBALMEM_SIZE - p;

	/*用户空间->内核空间*/
	  if (copy_from_user(mem + p, buff, count))
	    ret =  - EFAULT;
	  else
	  {
	    *offp += count;
	    ret = count;

	    //printk(KERN_INFO "written %d bytes(s) from %d\n", count, p);
	  }
  
	for(i=0;i<count;i++)
	{
		printk("%d",mem[i]);
		msleep(100); //delay 1ms
 	}
	printk("\n");
	return 0;
}

static int test_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	return 0;

}

static int test_release(struct inode *node, struct file *file)
{
	return 0;
}

/*
 * Set up the cdev structure for a device.
 */
static void test_setup_cdev(struct cdev *dev, int minor,
		struct file_operations *fops)
{
	int err, devno = MKDEV(test_major, minor);
    
	cdev_init(dev, fops);
	dev->owner = THIS_MODULE;
	dev->ops = fops;
	err = cdev_add (dev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk (KERN_NOTICE "Error %d adding test%d", err, minor);
}


/*
 * Our various sub-devices.
 */
/* Device 0 uses remap_pfn_range */
static struct file_operations test_remap_ops = {
	.owner   = THIS_MODULE,
	.open    = test_open,
	.release = test_release,
	.read    = test_read,
	.write   = test_write,
	.ioctl   = test_ioctl,	
};

/*
 * There's no need for us to maintain any
 * special housekeeping info, so we just deal with raw cdevs.
 */
static struct cdev TestDevs;

/*
 * Module housekeeping.
 */
static int test_init(void)
{
	int result;
	dev_t dev = MKDEV(test_major, 0);

	/* Figure out our device number. */
	if (test_major)
		result = register_chrdev_region(dev, 1, "nosem");
	else {
		result = alloc_chrdev_region(&dev, 0, 1, "nosem");
		test_major = MAJOR(dev);
	}
	if (result < 0) {
		printk(KERN_WARNING "test: unable to get major %d\n", test_major);
		return result;
	}
	if (test_major == 0)
		test_major = result;

	/* Now set up cdev. */
	test_setup_cdev(&TestDevs, 0, &test_remap_ops);
	printk("test device installed, with major %d\n", test_major);
	return 0;
}


static void test_cleanup(void)
{
	cdev_del(&TestDevs);
	unregister_chrdev_region(MKDEV(test_major, 0), 1);
	printk("test device uninstalled\n");
}


module_init(test_init);
module_exit(test_cleanup);