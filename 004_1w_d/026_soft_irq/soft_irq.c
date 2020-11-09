/*需要重新编译内核，一般不使用这种方式，略过*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>
#include <linux/timer.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>

MODULE_LICENSE("GPL");

int irqno[2] = {0};
int gpiono[2] = {0};
char irq_name[50] = {0};
char *key_name[2] = {"key2","key3"};

void soft_irq_action(struct softirq_action *action)
{
	int i=30;
	while(--i){
		printk("i = %d\n",i);
	}
}

irqreturn_t handle_irq_handler_f(int irqno, void *dev)
{
	raise_softirq(FAR_SOFTIRQ);
	return IRQ_HANDLED;
}


static int __init soft_irq_init(void)
{
	int i,ret;
	struct device_node *node;

	//2.注册底半部处理函数
	open_softirq(FAR_SOFTIRQ,soft_irq_action);
	
	node = of_find_node_by_path("/soft-key");
	if(node == NULL){
		printk("get soft key node error\n");
		return -EINVAL;
	}
	
	for(i=0; i<2; i++){//drivers/of/irq.c(解析函数的位置)
		irqno[i] = irq_of_parse_and_map(node,i);
		if(irqno[i] < 0){
			printk("irq parse error\n");
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
			printk("request irq  = %d error",irqno[i]);
			return -EAGAIN;
		}

		gpiono[i] = of_get_named_gpio(node,key_name[i],0);
		if(!gpio_is_valid(gpiono[i])){
			printk("get gpio number error\n");
			return -EAGAIN;
		}
		
	}

	
	return 0;
}
static void __exit soft_irq_exit(void)
{
	int i;
	for(i=0; i<2; i++){
		free_irq(irqno[i],NULL);
	}
}

module_init(soft_irq_init);
module_exit(soft_irq_exit);

