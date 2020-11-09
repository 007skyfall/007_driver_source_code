/*
 platform-hello{
    compatible = "hello2";
    reg = <0x114001e0 0x4>;
    interrupt-parent = <&gpf2>;
    interrupts = <1 0>;
};

*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif

struct resource *res[2];
int myresource_type[2] = {IORESOURCE_MEM,IORESOURCE_IRQ};

static int platform_drv_probe(struct platform_device *pdev)
{
	int i;
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	
	for(i=0; i<2; i++){
		res[i] = platform_get_resource(pdev, myresource_type[i] , 0);
		if(res[i] == NULL){
			printk("get resource error\n");
			return -EAGAIN;
		}
	}
	drv_pr("addr = %#x\n",res[0]->start);
	drv_pr("irq = %d\n",res[1]->start);
	
	return 0;
}

static int platform_drv_remove(struct platform_device *pdev)
{
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	
	return 0;
}

const struct of_device_id platform_pdrv_oftable[] = {
	{.compatible = "hello1",},
	{.compatible = "hello2",},
};	

struct platform_driver platform_drv = {
	.probe = platform_drv_probe,
	.remove = platform_drv_remove,
	.driver = {
		.name = "test",
		.owner = THIS_MODULE,
		.of_match_table = platform_pdrv_oftable,
	},
};

static int __init platform_drv_init(void)
{
	return platform_driver_register(&platform_drv);
}

static void __exit platform_drv_exit(void)
{
	platform_driver_unregister(&platform_drv);
}

module_init(platform_drv_init);
module_exit(platform_drv_exit);

