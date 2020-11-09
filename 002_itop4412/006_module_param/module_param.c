#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("SKYFALL");

static int module_arg1,module_arg2;
static int int_array[50];
static int int_num;

module_param(module_arg1,int,S_IRUSR);

module_param(module_arg2,int,S_IRUSR);

module_param_array(int_array,int,&int_num,S_IRUSR);


static int __init skyfall_init(void)
{
	int i;
	printk("%s,%d\n",__func__,__LINE__);
	
	printk(KERN_EMERG "module_arg1 is %d!\n",module_arg1);
	printk(KERN_EMERG "module_arg2 is %d!\n",module_arg2);
	
	for(i = 0; i < int_num; i++)
	{
		printk(KERN_EMERG "int_array[%d] is %d!\n",i,int_array[i]);
	}
		
	printk(KERN_EMERG "skyfall dirver enter!\n");
	
	return 0;
}

static void __exit skyfall_exit(void)
{
	
	printk("%s,%d\n",__func__,__LINE__);
	
	printk(KERN_EMERG "skyfall world exit!\n");

	return ;
}


module_init(skyfall_init);

module_exit(skyfall_exit);

