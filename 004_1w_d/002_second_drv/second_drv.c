#include <linux/init.h>
#include <linux/module.h>

extern int add(int a,int b);

static int __init demo_init(void)
{
	printk(KERN_ERR "hello deivce driver\n");

	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	printk("sum = %d\n",add(100,200));
	
	return 0;
}
static void __exit demo_exit(void)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return ;
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
