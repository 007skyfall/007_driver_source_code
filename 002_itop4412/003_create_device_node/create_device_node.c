#include <linux/init.h>
#include <linux/module.h>
/*驱动注册的头文件，包含驱动的结构体和注册和卸载的函数*/
#include <linux/platform_device.h>
/*注册杂项设备头文件*/
#include <linux/miscdevice.h>
/*注册设备节点的文件结构体*/
#include <linux/fs.h>

#define DRIVER_NAME "skyfall"
#define DEVICE_NAME "skyfall_007"
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("SKYFALL");

//long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
//驱动实际操作的实现
static long skyfall_ioctl( struct file *file, unsigned int cmd, unsigned long arg){

	printk("%s,%d\n",__func__,__LINE__);

	printk("cmd is %d,arg is %d\n",cmd,arg);
	
	return 0;
}

//int (*release) (struct inode *, struct file *);
//关闭该驱动的实现
static int skyfall_release(struct inode *inode, struct file *file){
	printk("%s,%d\n",__func__,__LINE__);

	printk(KERN_EMERG "skyfall release\n");

	return 0;
}

//int (*open) (struct inode *, struct file *);
//打开该驱动的实现
static int skyfall_open(struct inode *inode, struct file *file){
	printk("%s,%d\n",__func__,__LINE__);

	printk(KERN_EMERG "skyfall open\n");

	return 0;
}

//定义一个字符设备操作集合
static struct file_operations skyfall_ops = {
	.owner			 = THIS_MODULE,
	.open 	 		 = skyfall_open,
	.release 		 = skyfall_release,
	.unlocked_ioctl  = skyfall_ioctl,
};

//定义一个杂项设备结构体
static  struct miscdevice skyfall_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = DEVICE_NAME,
	//字符设备操作集合，需要提前定义一个，然后取地址
	.fops = &skyfall_ops,
};


static int skyfall_probe(struct platform_device *pdv){
	
	printk(KERN_EMERG "initialized\n");
	printk("%s,%d\n",__func__,__LINE__);
	
	//注册杂项设备，需要一个传一个杂项设备的结构体，所以需要提前定义一个，然后取地址
	misc_register(&skyfall_dev);
	
	return 0;
}

static int skyfall_remove(struct platform_device *pdv){
	
	printk(KERN_EMERG "\tremove\n");
	//注销杂项设备
	misc_deregister(&skyfall_dev);
	return 0;
}

static void skyfall_shutdown(struct platform_device *pdv){
	
	;
}

static int skyfall_suspend(struct platform_device *pdv,pm_message_t pmt){
	
	return 0;
}

static int skyfall_resume(struct platform_device *pdv){
	
	return 0;
}

struct platform_driver skyfall_driver = {
	.probe 		= skyfall_probe,
	.remove 	= skyfall_remove,
	.shutdown   = skyfall_shutdown,
	.suspend 	= skyfall_suspend,
	.resume     = skyfall_resume,
	.driver 	= {
					.name = DRIVER_NAME,
					.owner = THIS_MODULE,
	}
};


static int __init skyfall_init(void)
{
	int DriverState;//定义驱动状态，判断驱动是否注册成功
	
	printk(KERN_EMERG "skyfall driver enter!\n");
	printk("%s,%d\n",__func__,__LINE__);
	DriverState = platform_driver_register(&skyfall_driver);
	/*
	驱动一旦注册成功，会与设备进行匹配，匹配成功是由
	platform_match函数进行匹配，驱动调用probe初始化函数
	*/
	
	printk(KERN_EMERG "DriverState = %d\n",DriverState);
	
	return 0;
}


static void __exit skyfall_exit(void)
{
	printk(KERN_EMERG "skyfall driver exit!\n");
	
	platform_driver_unregister(&skyfall_driver);	
}

module_init(skyfall_init);
module_exit(skyfall_exit);
