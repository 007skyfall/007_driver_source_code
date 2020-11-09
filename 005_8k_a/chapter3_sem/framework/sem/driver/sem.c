/*
 * simple char device driver, no semaphore 
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>   /* printk() */
#include <linux/slab.h>     /* kmalloc() */
#include <linux/fs.h>       /* everything... */
#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>

#include <linux/device.h>
#include <asm/io.h>
#include <asm/semaphore.h> /*semaphore*/
#include <asm/uaccess.h>
#include <asm/arch-s3c2410/regs-gpio.h>

//DECLARE_MUTEX(sem);
static int test_major = 0;
module_param(test_major, int, 0);
MODULE_AUTHOR("Guo Cheng");
MODULE_LICENSE("Dual BSD/GPL");


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
	//Add your code here
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
		result = register_chrdev_region(dev, 1, "test");
	else {
		result = alloc_chrdev_region(&dev, 0, 1, "test");
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
EXPORT_SYMBOL(test_major);

