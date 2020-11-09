#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>


#define __DEBUG__  
#ifdef  __DEBUG__  
#define DEBUG(format,...) printk("File: "__FILE__", Line: %05d, Func: %s ++++ "format"\n", __LINE__,__func__, ##__VA_ARGS__)  
#else  
#define DEBUG(format,...)  
#endif 

MODULE_LICENSE("GPL");

int export_mult(int x,int y)
{
	DEBUG("enter export_mult");

	return x*y;
}


int export_div(int x,int y)
{
	DEBUG("enter export_mult");

	return x/y;
}

EXPORT_SYMBOL(export_mult);
EXPORT_SYMBOL(export_div);

static int __init export_module_init(void)
{
	DEBUG("enter export_module_init");
	
	return 0;
}

static void __exit export_module_exit(void)
{
	DEBUG("enter export_module_exit");
	
	return ;
}
module_init(export_module_init);
module_exit(export_module_exit);

MODULE_DESCRIPTION("This is a export_module demo!");
MODULE_AUTHOR("SKYFALL");
MODULE_ALIAS("export");