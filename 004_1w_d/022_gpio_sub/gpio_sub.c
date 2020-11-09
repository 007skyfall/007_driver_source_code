#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>

MODULE_LICENSE("GPL");

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif

int led[4];
int i;
static int __init gpio_sub_init(void)
{
	int ret;
	struct device_node *node;

	node = of_find_node_by_path("/a9-leds");
	if(node == NULL){
		drv_pr("get a9 leds node error\n");
		return -EINVAL;
	}
	
	led[0] = of_get_named_gpio(node, "led0", 0);
	led[1] = of_get_named_gpio(node, "led1", 0);
	led[2] = of_get_named_gpio(node, "led2", 0);
	led[3] = of_get_named_gpio(node, "led3", 0);

	for(i=0; i<4; i++){
		if(!gpio_is_valid(led[i])){
			drv_pr("gpio%d error\n",i);
			return -EAGAIN;
		}

		ret = gpio_request(led[i],NULL);
		if(ret){
			drv_pr("gpio %d request error",led[i]);
			return -EAGAIN;
		}
		ret = gpio_direction_output(led[i],1);
		if(ret){
			drv_pr("gpio %d output error",led[i]);
			return -EAGAIN;
		}
	}

	return 0;
}

static void __exit gpio_sub_exit(void)
{
	for(i=0; i<4; i++){
		gpio_free(led[i]);
	}
}

module_init(gpio_sub_init);
module_exit(gpio_sub_exit);

