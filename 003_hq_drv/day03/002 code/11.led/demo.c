#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/io.h>


/*物理地址--->dataesheet*/
#define GPF3CON  0X114001E0
#define GPF3DAT  0X114001E4

unsigned int *gpf3con = NULL;
unsigned int *gpf3dat = NULL;


#define NAME "demo"
int major = 0 , minor = 0;
/*
1.模块三要素
2.字符框架相关
	2.1 申请设备号
	2.2 cdev申请
	2.3 cdev初始化
	2.4 cdev注册
*/
int demo_open (struct inode *inode, struct file *file)
{
	writel( (readl(gpf3dat)&~(0x1<<5)) | (0x1<<5), gpf3dat);
	printk("%s,%d\n",__func__,__LINE__);
	return 0;
}

int demo_close (struct inode *inode, struct file *file)
{
	writel( (readl(gpf3dat)&~(0x1<<5)), gpf3dat);
	printk("%s,%d\n",__func__,__LINE__);
	return 0;
}

struct file_operations f_ops = {
	.owner = THIS_MODULE,
	.open = demo_open,
	.release = demo_close,
};
int __init demo_init(void)
{
	int ret = 0;
	struct resource *res;
	major = register_chrdev(major,NAME,&f_ops);
	if(major < 0){
		printk("register_chrdev fail ...%s,%d\n",__func__,__LINE__);
		return -EINVAL;
	}

	/*完成映射:物理--->虚拟*/
	res = request_mem_region(GPF3CON,8,"led4");
	if(res == NULL){
		printk("major :%d \t %s,%d\n",major ,__func__,__LINE__);
		goto ERR_STEP1;
	}
	gpf3con = ioremap((res->start),4);
	if(gpf3con == NULL){
		printk("major :%d \t %s,%d\n",major ,__func__,__LINE__);
		goto ERR_STEP1;
	}
	
	gpf3dat = ioremap((res->start+4),4);
	if(gpf3dat == NULL){
		printk("major :%d \t %s,%d\n",major ,__func__,__LINE__);
		goto ERR_STEP2;
	}

	writel((readl(gpf3con)&~(0xf<<20))|(0x1<<20),gpf3con);
	writel( (readl(gpf3dat)&~(0x1<<5)), gpf3dat);
	
	printk("major:%d \t %s,%d\n",major ,__func__,__LINE__);
	
	return 0;
	
ERR_STEP2:
	iounmap(gpf3con);
ERR_STEP1:
	unregister_chrdev(major,NAME);
	return ret;
}

void __exit demo_exit(void)
{
	release_mem_region(GPF3CON,8);
	iounmap(gpf3dat);
	iounmap(gpf3con);
	unregister_chrdev(major,NAME);
	printk("%s,%d\n",__func__,__LINE__);
}

module_init(demo_init);
module_exit(demo_exit);
MODULE_LICENSE("GPL");
