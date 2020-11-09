/*********************************************************************************************
#####         上海嵌入式家园-开发板商城         #####
#####                    www.embedclub.com                        #####
#####             http://embedclub.taobao.com               #####

* File：	hello_ext.c
* Author:	Hanson
* Desc：	hello module  with params
* History:	May 16th 2011
*********************************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Hanson He");

static char *whom = "world";
static int howmany = 1;


static int hello_init(void)
{
	int i;
	for(i=0;i<howmany;i++)
	{
        	printk(KERN_ALERT "Hello %s\n",whom);
	}
        return 0;
}
static void hello_exit(void)
{
        printk(KERN_ALERT " Hello world exit\n");
}
module_init(hello_init);
module_exit(hello_exit);
module_param(howmany, int, S_IRUGO);
module_param(whom, charp, S_IRUGO);
EXPORT_SYMBOL(howmany);
EXPORT_SYMBOL(whom);

