/******************************************************************************
#####         上海嵌入式家园-开发板商城         #####
#####                   		 www.embedclub.com                         #####
#####             		  http://embedclub.taobao.com                 #####
* File：		mem_malloc.c
* Author:		Hanson
* Desc：	malloc memory driver in kernel address space
* History:	May 16th 2011
*******************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/errno.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hanson He");
MODULE_DESCRIPTION("Memory alloc Module");
MODULE_ALIAS("malloc module");

char *buf1 = NULL;
char *buf2 = NULL;
char *buf3 = NULL;


static int alloc_init(void)
{
	buf1 = (unsigned char*) kmalloc(100,GFP_KERNEL);
	if (buf1 == NULL ) {
		printk("kmalloc error!\n");
		return -1;
	}
	printk("<1>kmalloc mem addr=%x\n", buf1);
	memset(buf1,0,100);
	strcpy(buf1,"<<< --- Kmalloc Mem OK! --- >>>");
	printk("<0>BUF 1 : %s\n",buf1);
	
	buf2 = (unsigned char*)get_zeroed_page(GFP_KERNEL);
	if (buf2 == NULL ) {
		printk("get_zeroed_page error!\n");
		return -1;
	}
	printk("<1>get_zeroed_page mem addr=%x\n", buf2);
	strcpy(buf2,"<<<--- Get Free Page OK! --- >>>");
	printk("<0>BUF 2 : %s\n",buf2);
	
	
	buf3 = (unsigned char*)vmalloc(1000000);
	if (buf3 == NULL ) {
		printk("vmalloc error!\n");
		return -1;
	}
	printk("<1>vmalloc mem addr=%x\n", buf3);
	memset(buf3,0,1000000);
	strcpy(buf3,"<<< --- Vmalloc Mem OK! --- >>>");
	printk("<0>BUF 3 : %s\n",buf3);

	return 0;
}


static void alloc_exit(void)
{
	kfree(buf1);
	free_page((unsigned long)buf2);
	vfree(buf3);
	printk("<0><<< --- Module Exit! --->>>\n");
}

module_init(alloc_init);
module_exit(alloc_exit);
