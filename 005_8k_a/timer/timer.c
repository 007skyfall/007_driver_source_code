#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/timer.h>  /*timer*/
#include <asm/uaccess.h>  /*jiffies*/

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hanson He");
MODULE_DESCRIPTION("Timer Module");
MODULE_ALIAS("timer module");

struct timer_list timer;

void timer_function(unsigned long para)
{
    printk("<0>Timer Expired and para is %d !!\n",para);	
}


static int timer_init(void)
{
	init_timer(&timer);
	timer.data = 5;
	timer.expires = jiffies + (5 * HZ);
	timer.function = timer_function;
	add_timer(&timer);
	
	return 0;
}


static void timer_exit(void)
{
	del_timer( &timer );
}

module_init(timer_init);
module_exit(timer_exit);
