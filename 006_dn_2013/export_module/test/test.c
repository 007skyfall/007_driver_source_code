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

extern int export_mult(int,int);
extern int export_div(int,int);


static int __init export_module_test_init(void)
{
	DEBUG("enter export_module_test_init");

	int mult = 0;
	mult = export_mult(8,2);
	printk("mult = %d\n",mult);
	
	return 0;
}

static void __exit export_module_test_exit(void)
{

	DEBUG("enter export_module_test_exit");
	
	int div = 0;
	div = export_div(8,2);
	printk("div = %d\n",div);
	
	return ;
}
module_init(export_module_test_init);
module_exit(export_module_test_exit);

MODULE_DESCRIPTION("This is a export_module test!");
MODULE_AUTHOR("SKYFALL");
MODULE_ALIAS("export test");

