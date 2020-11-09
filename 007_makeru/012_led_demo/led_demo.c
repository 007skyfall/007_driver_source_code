#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/io.h>

MODULE_LICENSE("GPL");
			
#define NAME "leds_demo"

//LED2 
#define     GPL2CON      0x11000100
#define     GPL2DAT      0x11000104

//LED3
#define 	GPK1CON 	0x11000060
#define 	GPK1DAT		0x11000064


void __iomem *gpl2con;
void __iomem *gpl2dat;

void __iomem *gpk1con;
void __iomem *gpk1dat;

int major = 0;

static int led_open(struct inode *inode, struct file *file)
{
	//点亮LED灯
	writel((0x1<<0),gpl2dat);
	writel((0x1<<1),gpk1dat);

	printk("%s,%d\n",__func__,__LINE__);
	return 0;
}

static int led_release(struct inode *inode, struct file *file)
{
	//熄灭LED灯
	writel(~(0x1<<0),gpl2dat);
	writel(~(0x1<<1),gpk1dat);

	printk("%s,%d\n",__func__,__LINE__);
	return 0;
}

struct file_operations f_ops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.release = led_release,

};
static int __init led_init(void)
{
	int ret = 0;
	printk("%s,%d\n",__func__,__LINE__);

	/*
	static inline int register_chrdev(unsigned int major, const char *name,
				  const struct file_operations *fops)
	*/
	major = register_chrdev(major,NAME,&f_ops);
	if(major < 0)
	{
		printk("%s,%dregister_chrdev fail...\n",__func__,__LINE__);
		return -EINVAL;
	}
	//ioremap (unsigned long phys_addr, unsigned long size)
	gpl2con = ioremap(GPL2CON,4);
	gpk1con = ioremap(GPK1CON,4);
	
	if(gpl2con == NULL || gpk1con == NULL)
	{
		printk("%s,%d ioremap gpl2con or gpK1con fail...\n",__func__,__LINE__);
		goto ERR_STEP1;
	}

	gpl2dat = ioremap(GPL2DAT,4);
	gpk1dat = ioremap(GPK1DAT,4);

	if(gpl2dat == NULL || gpk1dat == NULL )
	{
		printk("%s,%d ioremap gpL2dat or gpk1dat fail...\n",__func__,__LINE__);
		goto ERR_STEP2;
	}
	/*
	static inline u32 readl(const volatile void __iomem *addr)
	static inline void writel(unsigned int b, volatile void __iomem *addr)
	*/
	writel((readl(gpl2con)&~(0xf<<0))|(0x1<<0),gpl2con);
	writel((readl(gpk1con)&~(0xf<<4))|(0x1<<4),gpk1con);
	
	printk("%s,%d\n",__func__,__LINE__);
	return 0;

ERR_STEP2:
	//void iounmap(void *addr)
	iounmap(gpl2con);
	iounmap(gpk1con);

ERR_STEP1:
	//static inline void unregister_chrdev(unsigned int major, const char *name)
	unregister_chrdev(major,NAME);
	return ret;
}

static void __exit led_exit(void)
{
	iounmap(gpl2con);
	iounmap(gpl2dat);
	iounmap(gpk1con);
	iounmap(gpk1dat);

	unregister_chrdev(major,NAME);
	printk("%s,%d\n",__func__,__LINE__);

	return ;
}

module_init(led_init);
module_exit(led_exit);
