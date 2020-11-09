#include <linux/init.h>
#include <linux/module.h>

char tt = 10;
module_param(tt,byte,0664);
short aa = 10;
module_param(aa,short,0664);
MODULE_PARM_DESC(aa,"this is lcd light (0-255) val");
int bb = 20;
module_param(bb,int,0775);
MODULE_PARM_DESC(bb,"this is int val");

char * p = "hello world";
module_param(p,charp,0664);
MODULE_PARM_DESC(p,"this is char point val");

int ww[100] = {0};
int num;
module_param_array(ww,int,&num,0664);
MODULE_PARM_DESC(ww,"this is int array val");

char cbuf[100] = {0};
module_param_string(cbuf, cbuf, 100, 0775);
MODULE_PARM_DESC(cbuf,"this is char array val");

static int __init demo_init(void)
{
	int i;
	printk("tt = %c\n",tt);
	printk("aa = %d\n",aa);
	printk("bb = %d\n",bb);
	printk("p = %s\n",p);
	
	for(i=0; i<num; i++){
		printk("ww[%d] = %d\n",i,ww[i]);
	}
	printk("cbuf = %s\n",cbuf);
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
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
