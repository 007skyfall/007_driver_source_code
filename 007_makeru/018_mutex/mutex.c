#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");

#define NAME "demo_mutex"
int major = 0 , minor = 0;
struct mutex  mutex;

static int mutex_open(struct inode *inode, struct file *file)
{
	if(!mutex_trylock(&mutex))
	{
		printk("can not get mutex...\n");
		return -EBUSY;
	}
	printk("%s,%d\n",__func__,__LINE__);

	return 0;
}

static int mutex_close(struct inode *inode, struct file *file)
{
	mutex_unlock(&mutex);

	printk("%s,%d\n",__func__,__LINE__);

	return 0;
}


static struct file_operations f_ops = {
	.owner = THIS_MODULE,
	.open = mutex_open,
	.release = mutex_close,
};

static int __init mutex1_init(void)
{
	major = register_chrdev(major,NAME,&f_ops);
	if(major < 0)
	{
		printk("register_chrdev fail ...%s,%d\n",__func__,__LINE__);
		return -EINVAL;
	}

	mutex_init(&mutex);

	printk("major :%d \t %s,%d\n",major ,__func__,__LINE__);

	return 0;
}

static void __exit mutex1_exit(void)
{
	unregister_chrdev(major,NAME);

	printk("%s,%d\n",__func__,__LINE__);

	return ;
}

module_init(mutex1_init);
module_exit(mutex1_exit);
