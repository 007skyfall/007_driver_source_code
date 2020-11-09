/* 分配/设置/注册一个platform_driver */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/io.h>

MODULE_LICENSE("GPL");

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif

static int major;
static struct class *cls = NULL;
static volatile unsigned long *gpl2con = NULL;
static volatile unsigned long *gpl2dat = NULL;

static volatile unsigned long *gpk1con = NULL;
static volatile unsigned long *gpk1dat = NULL;

static int pin;

static int led_open(struct inode *inode, struct file *file)
{
	drv_pr("------------itop_leds_drv_open------------\n");
    
	/* 配置GPL2_0为输出 */
	*gpl2con &= ~((0xf<<(4*0)))|(0x01<<(4*0));
    
	/* 配置GPK1_1为输出 */
	*gpk1con |= ~((0xf<<(4*1)))|(0x01<<(4*1));

    return 0;
}


static ssize_t led_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;

	copy_from_user(&val, buf, count); //	copy_to_user();

	if (val == 1)
	{
		// 点灯
         *gpl2dat |= (0x01<<0);
        
		// LED3点灯
		*gpk1dat |= (0x01<<1);
	}
	else
	{
		// 灭灯
        *gpl2dat &= ~(0x01<<0);

        // LED3点灯
		*gpk1dat &= ~(0x01<<1);
	}
	
	return 0;
}


static struct file_operations led_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   led_open,     
	.write	=	led_write,	   
};

static int led_probe(struct platform_device *pdev)
{
	struct resource *res = NULL;
    int i;
    
	/* 根据platform_device的资源进行ioremap */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(res == NULL){
	drv_pr("%s,%d platform_get_resource fail...\n",__func__,__LINE__);
	return -ENOMEM;	
	}
	printk("res->start :%x , res->end :%x\n",res->start,res->end);
    
	gpl2con = ioremap(res->start, res->end - res->start + 1);
    
    res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if(res == NULL){
	drv_pr("%s,%d platform_get_resource fail...\n",__func__,__LINE__);
	return -ENOMEM;	
	}
	printk("res->start :%x , res->end :%x\n",res->start,res->end);
	gpl2dat = ioremap(res->start, res->end - res->start + 1);


	res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	if(res == NULL){
	drv_pr("%s,%d platform_get_resource fail...\n",__func__,__LINE__);
	return -ENOMEM;	
	}
	printk("res->start :%x , res->end :%x\n",res->start,res->end);
    
	gpk1con = ioremap(res->start, res->end - res->start + 1);
    
    res = platform_get_resource(pdev, IORESOURCE_MEM, 3);
	if(res == NULL){
	drv_pr("%s,%d platform_get_resource fail...\n",__func__,__LINE__);
	return -ENOMEM;	
	}
	printk("res->start :%x , res->end :%x\n",res->start,res->end);
	gpk1dat = ioremap(res->start, res->end - res->start + 1);

	/* 注册字符设备驱动程序 */

	drv_pr("led_probe, found led\n");

	major = register_chrdev(0, "myled", &led_fops);

	cls = class_create(THIS_MODULE, "myled");
    for(i=2; i<4; ++i){
    	device_create(cls, NULL, MKDEV(major, i), NULL, "led%d",i); /* /dev/led2 /dev/led3 */
    }
	
	*gpl2dat &= ~(0x01<<0);
	return 0;
}

static int led_remove(struct platform_device *pdev)
{
    int i;
    
	/* 卸载字符设备驱动程序 */
	/* iounmap */
	drv_pr("led_remove, remove led\n");
    for(i=2; i<4; i++){
    	device_destroy(cls, MKDEV(major, i));
    }
	class_destroy(cls);
	unregister_chrdev(major, "myled");
	
	iounmap(gpl2con);

    return 0;
}

struct platform_driver led_drv = {
	.probe		= led_probe,
	.remove		= led_remove,
	.driver		= {
		.name	= "myled",/*与dev里面的name要一样*/
	}
};


static int __init led_drv_init(void)
{
	platform_driver_register(&led_drv);
    
	return 0;
}

static void __exit led_drv_exit(void)
{
	platform_driver_unregister(&led_drv);

    return ;
}

module_init(led_drv_init);
module_exit(led_drv_exit);

