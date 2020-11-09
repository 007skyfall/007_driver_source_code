/*
  a9-leds{
  led2 = <&gpl2 0 0>;
  led3 = <&gpk1 1 0>;
};
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL");

int led[2];
int i;
struct timer_list mytimer;

void mytimer_function(unsigned long data)
{
	int led2_status, led3_status;
	led2_status = gpio_get_value(led[0]);
	led2_status = led2_status ? 0:1;
	gpio_set_value(led[0],led2_status);

	led3_status = gpio_get_value(led[1]);
	led3_status = led3_status ? 0:1;
	gpio_set_value(led[1],led3_status);
	
	mod_timer(&mytimer,jiffies+HZ);
}

static int __init gpio_sub_init(void)
{
	int ret;
	struct device_node *node;

	node = of_find_node_by_path("/a9-leds");
	if(node == NULL){
		printk("get a9 leds node error\n");
		return -EINVAL;
	}

	led[0] = of_get_named_gpio(node,"led2",0);
	led[1] = of_get_named_gpio(node,"led3",0);

	for(i=0; i<2; i++){
		if(!gpio_is_valid(led[i])){
			printk("gpio%d error\n",i);
			return -EAGAIN;
		}

		ret = gpio_request(led[i],NULL);
		if(ret){
			printk("gpio %d request error",led[i]);
			return -EAGAIN;
		}
		ret = gpio_direction_output(led[i],0);
		if(ret){
			printk("gpio %d output error",led[i]);
			return -EAGAIN;
		}
	}
	mytimer.expires = jiffies + HZ;
	mytimer.function = mytimer_function;
	mytimer.data = 0;
	init_timer(&mytimer);
	add_timer(&mytimer);

	return 0;
}

static void __exit gpio_sub_exit(void)
{
	del_timer(&mytimer);
	for(i=0; i<2; i++){
		gpio_free(led[i]);
	}

    return ;
}

module_init(gpio_sub_init);
module_exit(gpio_sub_exit);

