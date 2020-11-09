#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("skyfall");

static int __init hello_init(void)
{
	printk(KERN_EMERG "hello dirver enter!\n");
	return 0;
}

static void __exit hello_exit(void)
{
	printk(KERN_EMERG "hello dirver exit!\n");
	return ;
}

module_init(hello_init);
module_exit(hello_exit);
