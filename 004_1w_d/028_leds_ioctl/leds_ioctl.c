#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/device.h>
#include "myioctl.h"

MODULE_LICENSE("GPL");

#define NAME "leds_drv"

#define         GPL2CON         (0x11000100)
#define         GPL2DAT         (0x11000104)

#define 	    GPK1CON 	    (0x11000060)
#define 	    GPK1DAT		    (0x11000064)

int major;
int i;
char kbuf[128];
unsigned int *led2_con;
unsigned int *led2_dat;
unsigned int *led3_con;
unsigned int *led3_dat;
struct class *cls;
struct device *dev;
int which;

static int leds_drv_open(struct inode *inode, struct file *file)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	which = MINOR(inode->i_rdev);
	
	printk("------------which = %d------------\n",which);

    return 0;
}

static ssize_t leds_drv_read(struct file *file, char __user *ubuf, size_t size, loff_t *offs)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);

    return 0;
}

static ssize_t leds_drv_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offs)
{
	int ret;
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	if(size > sizeof(kbuf)) 
        size = sizeof(kbuf);
	ret = copy_from_user(kbuf,ubuf,size);
	switch(which){
		case 0:if(kbuf[0] == 1){
			*led2_dat |= (0x1<<0);
			}else{
				*led2_dat &= ~(0x1<<0);
			}break;
		
		case 1:if(kbuf[0] == 1){
			*led3_dat |= (0x1<<1);
			}else{
			*led2_dat &= ~(0x1<<0);
			}break;
	}
	
	return size;
}

static long leds_drv_ioctl(struct file *file, unsigned int cmd, unsigned long args)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	
	switch(cmd){
		case LED2_OP:{
				if(args){
					*led2_dat |= (0x1<<0);
				}else{
					*led2_dat &= ~(0x1<<0);
				}
			}break;
		
		case LED3_OP:{
				if(args){
					*led3_dat |= (0x1<<1);
				}else{
					*led3_dat &= ~(0x1<<1);
				}
			}break;
	}
	return 0;
}

static int leds_drv_close(struct inode *inode, struct file *file)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);

    return 0;
}

const struct file_operations fops = {
	.open           = leds_drv_open,
	.read           = leds_drv_read,
	.write          = leds_drv_write,
	.unlocked_ioctl = leds_drv_ioctl,
	.release        = leds_drv_close,
};

static int __init leds_drv_init(void)
{
	major = register_chrdev(0,NAME,&fops);
	if(major <= 0){
		printk("register chrdev error\n");
		return -EAGAIN;
	}
    
	led2_con = ioremap(GPL2CON,4);
	if(led2_con ==NULL){
		printk("ioremap led2 con error\n");
		return -EAGAIN;
	}
	led2_dat = ioremap(GPL2DAT,4);
	if(led2_dat ==NULL){
		printk("ioremap led2 dat error\n");
		return -EAGAIN;
	}
	
	led3_con = ioremap(GPK1CON,4);
	if(led3_con ==NULL){
		printk("ioremap led3 con error\n");
		return -EAGAIN;
	}
	led3_dat = ioremap(GPK1DAT,4);
	if(led3_dat ==NULL){
		printk("ioremap led3 dat error\n");
		return -EAGAIN;
	}
    
	//led init
	*led2_con &= ~(0xf<<0);
	*led2_con |= (0x1<<0);
	*led2_dat &= ~(0x1<<0);
	
	*led3_con &= ~(0xf<<4);
	*led3_con |= (0x1<<4);
	*led3_dat &= ~(0x1<<1);

	cls = class_create(THIS_MODULE,NAME);
	if(IS_ERR(cls)){
		printk("class create error\n");
		return PTR_ERR(cls);
	}
	for(i=0; i<2; i++){
		dev = device_create(cls,NULL,MKDEV(major,i),
			NULL,"leds_drv%d",i);
		if(IS_ERR(dev)){
			printk("device create error\n");
			return PTR_ERR(dev);
		}

	}

	return 0;
}

static void __exit leds_drv_exit(void)
{
	for(i=0;i<2;i++){
			device_destroy(cls,MKDEV(major,i));
	}
    
	class_destroy(cls);
	iounmap(led2_con);
	iounmap(led2_dat);
	iounmap(led3_con);
	iounmap(led3_dat);
	unregister_chrdev(major,NAME);

    return ;
}

module_init(leds_drv_init);
module_exit(leds_drv_exit);

