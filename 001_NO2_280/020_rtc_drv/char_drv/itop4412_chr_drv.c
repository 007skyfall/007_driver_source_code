#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/device.h>


#define CHAR_CNT   2

static struct cdev itop4412_chr_drv_cdev;
static struct cdev itop4412_chr_drv_cdev2;

static struct class *cls;

/* 1. 确定主设备号 */
static int major;

static int itop4412_chr_drv_open(struct inode *inode, struct file *file)
{
	printk("itop4412_chr_drv_open\n");
	
	return 0;
}

static int itop4412_chr_drv_open2(struct inode *inode, struct file *file)
{
	printk("itop4412_chr_drv_open2\n");
	
	return 0;
}

/* 2. 构造file_operations */
static struct file_operations itop4412_chr_drv_fops = {
	.owner = THIS_MODULE,
	.open  = itop4412_chr_drv_open,
};

static struct file_operations itop4412_chr_drv_fops2 = {
	.owner = THIS_MODULE,
	.open  = itop4412_chr_drv_open2,
};

static int __init itop4412_chr_drv_init(void)
{
	dev_t devid;
	
	/* 3. 告诉内核 */
#if 0
	major = register_chrdev(0, "itop4412_chr_drv", &itop4412_chr_drv_fops); 
	/* (major,  0), (major, 1), ..., (major, 255)都对应itop4412_chr_drv_fops */
#else
	if (major) {
		devid = MKDEV(major, 0);
		register_chrdev_region(devid, CHAR_CNT, "itop4412_chr_drv");  
	/* (major,0~1) 对应 itop4412_chr_drv_fops, (major, 2~255)都不对应itop4412_chr_drv_fops */
	} else {
		alloc_chrdev_region(&devid, 0, CHAR_CNT, "itop4412_chr_drv"); 
		/* (major,0~1) 对应 itop4412_chr_drv_fops, (major, 2~255)都不对应itop4412_chr_drv_fops */
		major = MAJOR(devid);                     
	}
	
	cdev_init(&itop4412_chr_drv_cdev, &itop4412_chr_drv_fops);
	cdev_add(&itop4412_chr_drv_cdev, devid, CHAR_CNT);

	devid = MKDEV(major, 2);
	register_chrdev_region(devid, 1, "itop4412_chr_drv2");
	cdev_init(&itop4412_chr_drv_cdev2, &itop4412_chr_drv_fops2);
	cdev_add(&itop4412_chr_drv_cdev2, devid, 1);
	
#endif

	cls = class_create(THIS_MODULE, "itop4412_chr_drv");
	device_create(cls, NULL, MKDEV(major, 0), NULL, "itop4412_chr_drv0"); 
	/* /dev/itop4412_chr_drv0 */

	device_create(cls, NULL, MKDEV(major, 1), NULL, "itop4412_chr_drv1"); 
	/* /dev/itop4412_chr_drv1 */

	device_create(cls, NULL, MKDEV(major, 2), NULL, "itop4412_chr_drv2"); 
	/* /dev/itop4412_chr_drv2 */
	
	device_create(cls, NULL, MKDEV(major, 3), NULL, "itop4412_chr_drv3"); 
	
	/* /dev/itop4412_chr_drv3 */
	
	return 0;
}

static void __exit itop4412_chr_drv_exit(void)
{
	device_destroy(cls, MKDEV(major, 0));
	device_destroy(cls, MKDEV(major, 1));
	device_destroy(cls, MKDEV(major, 2));
	device_destroy(cls, MKDEV(major, 3));

	class_destroy(cls);

	cdev_del(&itop4412_chr_drv_cdev);
	unregister_chrdev_region(MKDEV(major, 0), CHAR_CNT);
	
	cdev_del(&itop4412_chr_drv_cdev2);
	unregister_chrdev_region(MKDEV(major, 2), 1);

	return ;
}

module_init(itop4412_chr_drv_init);
module_exit(itop4412_chr_drv_exit);

MODULE_LICENSE("GPL");

