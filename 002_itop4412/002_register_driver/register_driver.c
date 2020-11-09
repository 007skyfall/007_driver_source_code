#include <linux/init.h>
#include <linux/module.h>
/*驱动注册的头文件，包含驱动的结构体和注册和卸载的函数*/
#include <linux/platform_device.h>

//DRIVER_NAME 需要和注册的设备名称相同
//这样才会调用platform_match与设备进行匹配
#define DRIVER_NAME "zhang"

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("SKYFALL");

static int skyfall_probe(struct platform_device *pdv){
	
	printk(KERN_EMERG "dirver is initialized\n");
	
	printk("%s,%d\n",__func__,__LINE__);
	
	return 0;
}

static int skyfall_remove(struct platform_device *pdv){
	
	return 0;
}

static void skyfall_shutdown(struct platform_device *pdv){
	
	;
}

static int skyfall_suspend(struct platform_device *pdv){
	
	return 0;
}

static int skyfall_resume(struct platform_device *pdv){
	
	return 0;
}

struct platform_driver skyfall_driver = {
	.probe 		= skyfall_probe,
	.remove 	= skyfall_remove,
	.shutdown 	= skyfall_shutdown,
	.suspend 	= skyfall_suspend,
	.resume 	= skyfall_resume,
	.driver 	= {
					.name = DRIVER_NAME,
					.owner = THIS_MODULE,
				}
};


static int __init skyfall_init(void)
{
	int DriverState;//定义驱动状态，判断驱动是否注册成功
	
	printk(KERN_EMERG "hello dirver enter!\n");
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
	printk(KERN_EMERG "hello dirver exit!\n");
	printk("%s,%d\n",__func__,__LINE__);

	platform_driver_unregister(&skyfall_driver);
	return ;	
}

module_init(skyfall_init);
module_exit(skyfall_exit);

