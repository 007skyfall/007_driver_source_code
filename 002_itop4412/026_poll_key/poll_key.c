#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <mach/regs-gpio.h>
#include <asm/io.h>
#include <linux/regulator/consumer.h>
#include <linux/delay.h>
#include <asm/uaccess.h>

#define DPRINTK(x...)	 printk("POLLKEY_CTL DEBUG:" x)
#define DRIVER_NAME 	"pollkey"

MODULE_LICENSE("Dual BSD/GPL");

static int key_gpios[] = {
	EXYNOS4_GPX1(1),
	EXYNOS4_GPX1(2),
};

static int pollkey_open(struct inode *inode,struct file *filp)
{
	DPRINTK("Device Opened Success!\n");
	return nonseekable_open(inode,filp);
}

static int pollkey_release(struct inode *inode,struct file *filp)
{
	DPRINTK("Device Closed Success!\n");
	return 0;
}

static ssize_t pollkey_read(struct file *filp, char __user *buff, size_t size, loff_t *ppos)
{
	unsigned char key_value[2];
	
	if(size != sizeof(key_value))
	{
		return -1;
	}
	
	key_value[0] = gpio_get_value(key_gpios[0]);
	key_value[1] = gpio_get_value(key_gpios[1]);
	
	copy_to_user(buff,key_value,sizeof(key_value));
	
	return 0;
}

static struct file_operations pollkey_ops = {
	.owner 	= THIS_MODULE,
	.open 	= pollkey_open,
	.release= pollkey_release,
	.read 	= pollkey_read,
};

static struct miscdevice pollkey_dev = {
	.minor	= MISC_DYNAMIC_MINOR,
	.fops	= &pollkey_ops,
	.name	= "pollkey",
};


static int pollkey_probe(struct platform_device *pdev)
{
	printk("%s:%d\n",__func__,__LINE__);
	
	int ret,i;
	#if 1
	char *banner = "pollkey Initialize\n";
	printk(banner);
	printk("%s:%d\n",__func__,__LINE__);
	for(i=0;i<2;i++)
	{
		ret = gpio_request(key_gpios[i],"key_gpio");
		s3c_gpio_cfgpin(key_gpios[i],S3C_GPIO_INPUT); 
		s3c_gpio_setpull(key_gpios[i],S3C_GPIO_PULL_NONE);
	}
	#endif
	ret = misc_register(&pollkey_dev);
	return 0;
}

static int pollkey_remove (struct platform_device *pdev)
{
	misc_deregister(&pollkey_dev);	
	return 0;
}

static int pollkey_suspend (struct platform_device *pdev, pm_message_t state)
{
	DPRINTK("pollkey suspend:power off!\n");
	return 0;
}

static int pollkey_resume (struct platform_device *pdev)
{
	DPRINTK("pollkey resume:power on!\n");
	return 0;
}

struct platform_driver pollkey_driver = {
	.probe   = pollkey_probe,
	.remove  = pollkey_remove,
	.suspend = pollkey_suspend,
	.resume  = pollkey_resume,
	.driver  = {
					.name  = DRIVER_NAME,
					.owner = THIS_MODULE,
					},
};

static int __init pollkey_init(void)
{
	int DriverState;
	printk("%s,%d\n",__func__,__LINE__);
	printk(KERN_EMERG "pollkey_init enter!\n");
	DriverState = platform_driver_register(&pollkey_driver);
	printk(KERN_EMERG "\tDriverState is %d\n",DriverState);
	return 0;
}

static void __exit pollkey_exit(void)
{
	platform_driver_unregister(&pollkey_driver);
	printk("%s:%d\n",__func__,__LINE__);
	return ;
}

module_init(pollkey_init);
module_exit(pollkey_exit);
