#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/device.h>

MODULE_LICENSE("GPL");

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif
/*
*   UART_RING========================GPX1_1=====HOME
*   SIM_DET==========================GPX1_2=====BACK
*   GYPO_INT=========================GPX3_3=====SLEEP
*   KP_ROW1=========================GPX2_1=======V+
*   KP_ROW0=========================GPX2_0=======V-
*
*/
#define GPX1  		(0x11000C20)
#define GPX2  		(0x11000C40)
#define GPX3  		(0x11000C60)
#define GPIO_SIZE  		8


static struct class     *query_key_class    = NULL;
static struct device	*query_key_device   = NULL;

volatile unsigned long  *gpx1con            = NULL;
volatile unsigned long  *gpx1dat            = NULL;       
volatile unsigned long  *gpx2con            = NULL;
volatile unsigned long  *gpx2dat            = NULL;
volatile unsigned long  *gpx3con            = NULL;
volatile unsigned long  *gpx3dat            = NULL;

int major   = 0;
int counter = 0;

static int query_key_open(struct inode *inode, struct file *file)
{
	/* 配置GPX1_1,GPX1_2为输入引脚 */
	*gpx1con &= ~((0xf<<(1*4)) | (0x0<<(1*4)) | (0xf<<(2*4)) | (0x0<<(2*4)));
	
	/* 配置GPX2_0,GPX2_1为输入引脚 */
	*gpx2con &= ~((0xf<<(0*4)))|(0x0<<(0*4) | (0xf<<(1*4)))|(0x0<<(1*4));

	/* 配置GPX3_3为输入引脚  */
	*gpx3con &= ~((0xf<<(3*4)))|(0x0<<(3*4));

	return 0;
}

static ssize_t query_key_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	/* 返回5个引脚的电平 */
	unsigned int key_val     = 0;
    unsigned int key_vals[5] = { 0 };
	int regval, n;

#if 0
////////////////////HOME=====GPX1_1//////////////////////////////////////
	regval = *gpx1dat;
	key_val = (regval & (0x1<<1)) ? 1 : 0;
	copy_to_user(buf, &key_val, sizeof(key_val));
//////////////////////////////////////////////////////////////////////////
#endif 

#if 0
////////////////////BACK=GPX1_2////////////////////////////////////////////	
	regval = *gpx1dat;
	key_val = (regval & (0x1<<2)) ? 1 : 0;
	copy_to_user(buf, &key_val, sizeof(key_val));
//////////////////////////////////////////////////////////////////////////
#endif 

#if 0
////////////////////SLEEP=====GPX3_3////////////////////////////////////////////	
	regval = *gpx3dat;
	key_val = (regval & (0x1<<3)) ? 1 : 0;
	copy_to_user(buf, &key_val, sizeof(key_val));
//////////////////////////////////////////////////////////////////////////
#endif 

#if 0
////////////////////VOL-===GPX2_0////////////////////////////////////////////	
	regval = *gpx2dat;
	key_val = (regval & (0x1<<0)) ? 1 : 0;
	copy_to_user(buf, &key_val, sizeof(key_val));
//////////////////////////////////////////////////////////////////////////
#endif 

#if 0
////////////////////VOL+===GPX2_1////////////////////////////////////////////	
	regval = *gpx2dat;
	key_val = (regval & (0x1<<1)) ? 1 : 0;
    
//    unsigned long copy_to_user(void __user *to, const void *from, unsigned long n)
	copy_to_user(buf, &key_val, sizeof(key_val));
//////////////////////////////////////////////////////////////////////////
#endif

#if 1
    n = ARRAY_SIZE(key_vals);
//    drv_pr("n = %d!\n", n);
    
    /* 读GPX1_1,GPX1_2 */
	regval = *gpx1dat;
	key_vals[0] = (regval & (0x1<<1)) ? 1 : 0;
	key_vals[1] = (regval & (0x1<<2)) ? 1 : 0;
	
	/* 读GPX2_0,GPX2_1 */
	regval = *gpx2dat;
	key_vals[2] = (regval & (0x1<<0)) ? 1 : 0;
	key_vals[3] = (regval & (0x1<<1)) ? 1 : 0;

	/* 读GPX3_3 */
	regval = *gpx3dat;
	key_vals[4] = (regval & (0x1<<3)) ? 1 : 0;

    copy_to_user(buf, key_vals, sizeof(key_vals));

#endif

	return n;
}

static struct file_operations query_key_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   query_key_open,     
	.read	=	query_key_read,	   
};

static int __init query_key_init(void)
{
    
    drv_pr("%s : %s : %d\n",__FILE__, __func__, __LINE__);
    
	major = register_chrdev(0, "query_key", &query_key_fops);
	query_key_class = class_create(THIS_MODULE, "query_key_drv");

	query_key_device = device_create(query_key_class, NULL, MKDEV(major, 0), NULL, "button"); /* /dev/button*/

	gpx1con = (volatile unsigned long *)ioremap(GPX1, GPIO_SIZE);
	gpx1dat = gpx1con + 1;

	gpx2con = (volatile unsigned long *)ioremap(GPX2, GPIO_SIZE);
	gpx2dat = gpx2con + 1;
	
	gpx3con = (volatile unsigned long *)ioremap(GPX3, GPIO_SIZE);
	gpx3dat = gpx3con + 1;

	return 0;
}

static void __exit query_key_exit(void)
{
	iounmap(gpx1con);
	iounmap(gpx2con);
	iounmap(gpx3con);

	device_destroy(query_key_class,MKDEV(major, 0));
	class_destroy(query_key_class);

	unregister_chrdev(major, "query_key");
	
	return ;
}

module_init(query_key_init);
module_exit(query_key_exit);

