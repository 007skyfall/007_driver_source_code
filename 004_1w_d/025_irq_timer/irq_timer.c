/*有问题，未解决！  20200603*/
/*
 itop4412-key {
 		   compatible = "irq_gpios";
 		   status = "okay";
 		   home = <&gpx1 1 0>;
 		   back = <&gpx1 2 0>;																				  
 	 };

*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>
#include <linux/timer.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("seedling");
MODULE_DESCRIPTION("itop4412_of_gpio_key");

#define DRIVER_NAME "irq_gpios"

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define printk(...)
#endif

struct timer_list mytimer;
int irqno[2]        = {0};
int gpiono[2]       = {0};
char irq_name[50]   = {0};
char *key_name[2]   = {"home", "back"};

void irq_timer_function(unsigned long data)
{
	int status_home = gpio_get_value(gpiono[0]);
	int status_back = gpio_get_value(gpiono[1]);
	
	if(status_home == 0){
		drv_pr("irqno = %d,home.......\n",irqno[0]);
	}
	if(status_back== 0){
		drv_pr("irqno = %d,back.......\n",irqno[1]);
	}
}

irqreturn_t handle_irq_handler_f(int irqno, void *dev)
{
	mod_timer(&mytimer,jiffies+2);
	
	return IRQ_HANDLED;
}


static int gpio_probe(struct platform_device * pdev)
	{
		int i,ret;
		struct device_node *node;
		printk("%s,%d\n",__func__,__LINE__);
		mytimer.expires = jiffies + 2;
		mytimer.function = irq_timer_function;
		mytimer.data = 0;
		init_timer(&mytimer);
		add_timer(&mytimer);
	
		node = of_find_node_by_path("/itop4412-key");
		if(node == NULL){
			drv_pr("get  itop4412-key node error\n");
			return -EINVAL;
		}
		
		for(i=0; i<2; i++){//drivers/of/irq.c(解析函数的位置)
			irqno[i] = irq_of_parse_and_map(node,i);
			
			drv_pr("irqno[i] = %d\n", irqno[i]);
			if(irqno[i] <= 0){
				drv_pr("irq parse error\n");
				return -EAGAIN;
			}
			
			
			memset(irq_name,0,sizeof(irq_name));
			if(i == 0){
				memcpy(irq_name,"interrupt-gpx1_1",sizeof(irq_name));
			}else{
				memcpy(irq_name,"interrupt-gpx1_2",sizeof(irq_name));
			}
	
			ret = request_irq(irqno[i],handle_irq_handler_f,
					IRQF_TRIGGER_FALLING,irq_name,NULL);
			if(ret){
				drv_pr("request irq  = %d error",irqno[i]);
				return -EAGAIN;
			}
	
			gpiono[i] = of_get_named_gpio(node,key_name[i],0);
			if(!gpio_is_valid(gpiono[i])){
				drv_pr("get gpio number error\n");
				return -EAGAIN;
			}
			
		}
	
		return 0;
	}


static int gpio_remove(struct platform_device * pdev)
{
    printk(KERN_ALERT "gpio_remove!\n");

    return 0;
}

static const struct of_device_id of_gpio_dt_match[] = {
    {.compatible = DRIVER_NAME},
    {},
};

MODULE_DEVICE_TABLE(of,of_gpio_dt_match);

static struct platform_driver gpio_driver = {
    .probe  = gpio_probe,
    .remove = gpio_remove,
    .driver = {
        .name = DRIVER_NAME,
        .owner = THIS_MODULE,
        .of_match_table = of_gpio_dt_match,
        },
};

static int __init itop4412_irq_init(void)
{
    printk("%s,%d\n",__func__,__LINE__);

    platform_driver_register(&gpio_driver);

    return 0;
}

static void __exit itop4412_irq_exit(void)
{
    printk("%s,%d\n",__func__,__LINE__);

	int i;
	for(i=0; i<2; i++){
		free_irq(irqno[i],NULL);
	}
    del_timer(&mytimer);
	
    platform_driver_unregister(&gpio_driver);

    return ;
}

module_init(itop4412_irq_init);
module_exit(itop4412_irq_exit);

