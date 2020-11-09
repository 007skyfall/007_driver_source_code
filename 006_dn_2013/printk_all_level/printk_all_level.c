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

static int __init printk_all_level_init(void)
{
	
	DEBUG("enter printk_all_level_init");
	
	printk(KERN_EMERG   "0 (KERN_EMERG)		    system is unusable");
	printk(KERN_ALERT   "1 (KERN_ALERT)		    action must be taken immediately");
	printk(KERN_CRIT    "2 (KERN_CRIT)		    critical conditions");
	printk(KERN_ERR     "3 (KERN_ERR)		    error conditions");
	printk(KERN_WARNING "4 (KERN_WARNING)	    warning conditions");
	printk(KERN_NOTICE  "5 (KERN_NOTICE)		normal but significant condition");
	printk(KERN_INFO    "6 (KERN_INFO)		    informational");
	printk(KERN_DEBUG   "7 (KERN_DEBUG)		    debug-level messages");		
		
	return 0;
}

static void __exit printk_all_level_exit(void)
{
	DEBUG("enter printk_all_level_exit");
	
	printk(KERN_EMERG   "0 (KERN_EMERG)		    system is unusable");
	printk(KERN_ALERT   "1 (KERN_ALERT)		    action must be taken immediately");
	printk(KERN_CRIT    "2 (KERN_CRIT)		    critical conditions");
	printk(KERN_ERR     "3 (KERN_ERR)		    error conditions");
	printk(KERN_WARNING "4 (KERN_WARNING)	    warning conditions");
	printk(KERN_NOTICE  "5 (KERN_NOTICE)		normal but significant condition");
	printk(KERN_INFO    "6 (KERN_INFO)		    informational");
	printk(KERN_DEBUG   "7 (KERN_DEBUG)		    debug-level messages");		
	
	return ;
}
module_init(printk_all_level_init);
module_exit(printk_all_level_exit);

MODULE_DESCRIPTION("This is a printk_all_level demo!");
MODULE_AUTHOR("SKYFALL");
MODULE_ALIAS("printk_all_level");
