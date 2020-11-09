#include <linux/init.h>
#include <linux/module.h>

int add(int a,int b)
{
	return (a+b);
}

EXPORT_SYMBOL_GPL(add);

static int __init add_init(void)
{
	return 0;
}

static void __exit add_exit(void)
{
	return ;
}

module_init(add_init);
module_exit(add_exit);

MODULE_LICENSE("GPL");
