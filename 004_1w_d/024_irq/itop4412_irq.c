/*
itop4412-key{
interrupt-parent = <&gpx1>;
interrupts = <1 0>,<2 0>; 
};  
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>

MODULE_LICENSE("GPL");

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ drv_pr("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif


int irqno[2] = {0};
char irq_name[50] = {0};

irqreturn_t handle_irq_handler_f(int irqno, void *dev)
{
	int i = (int)dev;
	drv_pr("i = %d\n",i);
	if(i == 0){
		drv_pr("irq = %d,home............\n",irqno);
	}else{
		drv_pr("irq = %d,back............\n",irqno);
	}

	return IRQ_HANDLED;
}

static int __init itop4412_irq_init(void)
{
	int i,ret;

	struct device_node *node;
	node = of_find_node_by_path("/itop4412-key");
	if(node == NULL){
		drv_pr("get itop4412 key node error\n");
		return -EINVAL;
	}
	
	for(i=0; i<2; i++){ //drivers/of/irq.c(解析函数的位置)
		irqno[i] = irq_of_parse_and_map(node,i);
		if(irqno[i] < 0){
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
				IRQF_TRIGGER_FALLING,irq_name,(void *)i);
		if(ret){
			drv_pr("request irq  = %d error",irqno[i]);
			return -EAGAIN;
		}
		
	}
	
	return 0;
}

static void __exit itop4412_irq_exit(void)
{
	int i;
	for(i=0; i<2; i++){
		free_irq(irqno[i],(void *)i);
	}
}

module_init(itop4412_irq_init);
module_exit(itop4412_irq_exit);

