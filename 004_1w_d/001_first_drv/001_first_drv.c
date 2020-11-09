#include <linux/init.h>
#include <linux/module.h>

static int __init demo_init(void)
{
	printk(KERN_ERR "hello deivce driver\n");
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}
static void __exit demo_exit(void)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
