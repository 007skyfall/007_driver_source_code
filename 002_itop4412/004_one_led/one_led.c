#include <linux/init.h>
#include <linux/module.h>
/*驱动注册的头文件，包含驱动的结构体和注册和卸载的函数*/
#include <linux/platform_device.h>
/*注册杂项设备头文件*/
#include <linux/miscdevice.h>
/*注册设备节点的文件结构体*/
#include <linux/fs.h>
/*Linux中申请GPIO的头文件*/
#include <linux/gpio.h>
/*三星平台的GPIO配置函数头文件*/
/*三星平台EXYNOS系列平台，GPIO配置参数宏定义头文件*/
#include <plat/gpio-cfg.h>
#include <mach/gpio.h>
/*三星平台4412平台，GPIO宏定义头文件*/
#include <mach/gpio-exynos4.h>

#define DRIVER_NAME "skyfall"
#define DEVICE_NAME "skyfall_007"


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("SKYFALL");

static long skyfall_ioctl( struct file *files, unsigned int cmd, unsigned long arg){
	
	printk("cmd is %d,arg is %d\n",cmd,arg);

	//cmd=0，led灯熄灭；cmd=1，led灯点亮；
	//arg=1,在应用写死了，这是一个演示，两个灯时可以传参
	if(cmd > 1)
	{
		printk(KERN_EMERG "cmd is 0 or 1\n");
	}
	if(arg > 1)
	{
		printk(KERN_EMERG "arg is only 1\n");
	}
	
	gpio_set_value(EXYNOS4_GPL2(0),cmd);
	gpio_set_value(EXYNOS4_GPK1(1),!cmd);
	
	return 0;
}

static int skyfall_release(struct inode *inode, struct file *file){
	
	printk("%s,%d\n",__func__,__LINE__);

	printk(KERN_EMERG "skyfall release!\n");

	return 0;
}

static int skyfall_open(struct inode *inode, struct file *file){
	
	printk("%s,%d\n",__func__,__LINE__);

	printk(KERN_EMERG "skyfall open!\n");

	return 0;
}

static struct file_operations skyfall_ops = {
	.owner			 = THIS_MODULE,
	.open 			 = skyfall_open,
	.release		 = skyfall_release,
	.unlocked_ioctl  = skyfall_ioctl,
};

static  struct miscdevice skyfall_dev = {
	.minor		 = MISC_DYNAMIC_MINOR,
	.name 		 = DEVICE_NAME,
	.fops 		 = &skyfall_ops,
};


static int skyfall_probe(struct platform_device *pdv){
	
	int ret[2] = {0};
	int i = 0;
	
	printk("%s,%d\n",__func__,__LINE__);

	printk(KERN_EMERG "initialized\n");
	
	ret[0] = gpio_request(EXYNOS4_GPL2(0),"LED2");
	ret[1] = gpio_request(EXYNOS4_GPK1(1),"LED3");
	for(i = 0; i < 2; i++){
			if(ret[i] < 0)
			{
				printk(KERN_EMERG "gpio_request EXYNOS4 GPIO failed!\n");
				return ret[i];
			}
	}
	s3c_gpio_cfgpin(EXYNOS4_GPL2(0),S3C_GPIO_OUTPUT);
	s3c_gpio_cfgpin(EXYNOS4_GPK1(1),S3C_GPIO_OUTPUT);
	
	gpio_set_value(EXYNOS4_GPL2(0),0);
	gpio_set_value(EXYNOS4_GPK1(1),0);
	
	misc_register(&skyfall_dev);
	
	return 0;
}

static int skyfall_remove(struct platform_device *pdv){

	
	printk("%s,%d\n",__func__,__LINE__);

	printk(KERN_EMERG "skyfall_remove!\n");

	misc_deregister(&skyfall_dev);

	return 0;
}

static void skyfall_shutdown(struct platform_device *pdv){
	
	return ;
}

static int skyfall_suspend(struct platform_device *pdv,pm_message_t pmt){
	
	return 0;
}

static int skyfall_resume(struct platform_device *pdv){
	
	return 0;
}

struct platform_driver skyfall_driver = {
	.probe 			= skyfall_probe,
	.remove 		= skyfall_remove,
	.shutdown	    = skyfall_shutdown,
	.suspend 		= skyfall_suspend,
	.resume 		= skyfall_resume,
	.driver 		= {
						.name  = DRIVER_NAME,
						.owner = THIS_MODULE,
							}
};


static int __init skyfall_init(void)
{
	int DriverState;
	
	printk("%s,%d\n",__func__,__LINE__);

	printk(KERN_EMERG "skyfall driver enter!\n");

	DriverState = platform_driver_register(&skyfall_driver);
	
	printk(KERN_EMERG "DriverState = %d\n",DriverState);

	return 0;
}


static void skyfall_exit(void)
{
	
	printk("%s,%d\n",__func__,__LINE__);

	printk(KERN_EMERG "skyfall driver exit!\n");
	
	platform_driver_unregister(&skyfall_driver);

	return ;	
}

module_init(skyfall_init);
module_exit(skyfall_exit);
