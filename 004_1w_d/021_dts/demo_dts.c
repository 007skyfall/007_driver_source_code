/*
#if 0

 demo@114001e0{
	a-string-property = "helloworld";
	a-string-list-property = "list1","list2","list3";
	a-array-property = <0x123456 0x23456 0x5678>;
	a-mux-property = "duangduangduang",<0x876543>,[00 11 22];
}; 

#else

a-string-property = helloworld,len = 11
list1 = list1,len = 18
list2 = list2,len = 18
list3 = list3,len = 18
int1 = 0x123456,len = 12
int2 = 0x23456,len = 12
int3 = 0x5678,len = 12
char = duangduangduang,len = 23
int = 0x3,len = 23
int = 0x0,len = 23
int = 0x11,len = 23
int = 0x22,len = 23

#endif
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>

MODULE_LICENSE("GPL");

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif

static int __init demo_dts_init(void)
{
	int len,i;
	struct device_node *node = NULL;
	struct property * pro    = NULL;
	
	node = of_find_node_by_path("/demo@114001e0");
	if(node == NULL){
		drv_pr("get node demo error\n");
		return -EINVAL;
	}

	pro = of_find_property(node,"a-string-property",&len);
	if(pro == NULL){
		drv_pr("get a string property error\n");
		return -EINVAL;
	}

	printk("a-string-property = %s,len = %d\n",(char *)pro->value,len);


	pro = of_find_property(node,"a-string-list-property",&len);
	if(pro == NULL){
		drv_pr("get a string property error\n");
		return -EINVAL;
	}
	
	printk("list1 = %s,len = %d\n",(char *)pro->value,len);
	printk("list2 = %s,len = %d\n",(char *)pro->value+6,len);
	printk("list3 = %s,len = %d\n",(char *)pro->value+12,len);


	pro = of_find_property(node,"a-array-property",&len);
	if(pro == NULL){
		drv_pr("get a string property error\n");
		return -EINVAL;
	}
	
	printk("int1 = %#x,len = %d\n",be32_to_cpup((__be32 *)pro->value),len);
	printk("int2 = %#x,len = %d\n",be32_to_cpup((__be32 *)pro->value+1),len);
	printk("int3 = %#x,len = %d\n",be32_to_cpup((__be32 *)pro->value+2),len);

	
	pro = of_find_property(node,"a-mux-property",&len);
	if(pro == NULL){
		drv_pr("get a string property error\n");
		return -EINVAL;
	}
	
	printk("char = %s,len = %d\n",(char *)pro->value,len);
	printk("int = %#x,len = %d\n",be32_to_cpup((__be32 *)pro->value+4),len);
	for(i=0; i<3; i++){
		printk("int = %#x,len = %d\n",*((char *)pro->value+20+i),len);
	}
	
	return 0;
}
static void __exit demo_dts_exit(void)
{
	return ;
}

module_init(demo_dts_init);
module_exit(demo_dts_exit);

