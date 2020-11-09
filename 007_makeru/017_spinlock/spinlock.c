#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/spinlock.h>

MODULE_LICENSE("GPL");

#define NAME "spinlock_demo"
int major = 0 , minor = 0;
spinlock_t lock;
int flag = 0;

static int spinlock_open(struct inode *inode, struct file *file)
{
//static inline void spin_lock(spinlock_t *lock)
	spin_lock(&lock);
	if(flag)
	{
		spin_unlock(&lock);
		printk("the device open already...\n");
		return -EBUSY;
	}
	flag = 1;
//static inline void spin_unlock(spinlock_t *lock)
	spin_unlock(&lock);

	printk("%s,%d\n",__func__,__LINE__);

	return 0;
}

static int spinlock_close (struct inode *inode, struct file *file)
{
	spin_lock(&lock);
	flag = 0;
	spin_unlock(&lock);
	
	printk("%s,%d\n",__func__,__LINE__);

	return 0;
}


static struct file_operations f_ops = {
	.owner = THIS_MODULE,
	.open = spinlock_open,
	.release = spinlock_close,
};
	
static int __init spinlock_init(void)
{
	major = register_chrdev(major,NAME,&f_ops);
	if(major < 0)
	{
		printk("register_chrdev fail ...%s,%d\n",__func__,__LINE__);

		return -EINVAL;
	}

	spin_lock_init(&lock);
	printk("major :%d \t %s,%d\n",major ,__func__,__LINE__);

	return 0;
}

static void __exit spinlock_exit(void)
{
	unregister_chrdev(major,NAME);

	printk("%s,%d\n",__func__,__LINE__);

	return ;
}

module_init(spinlock_init);
module_exit(spinlock_exit);
