#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif

#define NAME 		"my_cdev"
int major = 0;

static ssize_t my_cdev_read(struct file *file, char __user *ubuf, size_t size, loff_t *offs)
{
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;
}

static ssize_t my_cdev_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offs)
{
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;
}

static int my_cdev_open(struct inode *inode, struct file *file)
{
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;
}

static int my_cdev_close(struct inode *inode, struct file *file)
{
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;
}

const struct file_operations fops = {
	.open    = my_cdev_open,
	.read    = my_cdev_read,
	.write   = my_cdev_write,
	.release = my_cdev_close,
};

static int __init my_cdev_init(void)
{
	major = register_chrdev(0,NAME,&fops);
	if(major <= 0){
		drv_pr("register chrdev error\n");
		return -EAGAIN;
	}

	return 0;
}

static void __exit my_cdev_exit(void)
{
	unregister_chrdev(major,NAME);

	return ;
}

module_init(my_cdev_init);
module_exit(my_cdev_exit);

MODULE_LICENSE("GPL");

