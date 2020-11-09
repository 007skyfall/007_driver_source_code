#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "myioctl.h"

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif


#define NAME 			"my_ioctl"
char kbuf[128] = { 0 };
int major = 0;

static ssize_t my_ioctl_read(struct file *file, char __user *ubuf, size_t size, loff_t *offs)
{
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;
}

static ssize_t my_ioctl_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offs)
{
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;
}

static int my_ioctl_open(struct inode *inode, struct file *file)
{
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;
}


static long my_ioctl_ioctl(struct file *file, unsigned int cmd, unsigned long args)
{
	int ret;
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	
	switch(cmd)
		{
			case ACCESS_INT:
								printk("args = %ld\n",args);
								break;
			case ACCESS_STR_W:
							{
								memset(kbuf,0,sizeof(kbuf));
								ret = copy_from_user(kbuf,(void *)args,sizeof(kbuf));
								if(ret){
									printk("access data to kernel error\n");
									return -EINVAL;
								}
								printk("kdata = %s\n",kbuf);
							}
								break;
			case ACCESS_STR_R:
							{
								ret = copy_to_user((void *)args,kbuf,sizeof(kbuf));
								if(ret){
									printk("access data from kernel error\n");
									return -EINVAL;
								}
							}
								break;
		}
	
	return 0;
}

static int my_ioctl_close(struct inode *inode, struct file *file)
{
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;

}

const struct file_operations fops = {
	.open 				= my_ioctl_open,
	.read 				= my_ioctl_read,
	.write 				= my_ioctl_write,
	.unlocked_ioctl 	= my_ioctl_ioctl,
	.release 			= my_ioctl_close,
};

static int __init my_ioctl_init(void)
{
	major = register_chrdev(0,NAME,&fops);
	if(major <= 0){
		printk("register chrdev error\n");
		return -EAGAIN;
	}

	return 0;
}
static void __exit my_ioctl_exit(void)
{
	unregister_chrdev(major,NAME);

	return ;
}

module_init(my_ioctl_init);
module_exit(my_ioctl_exit);

MODULE_LICENSE("GPL");

