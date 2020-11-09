#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/semaphore.h>

MODULE_LICENSE("GPL");

#define NAME "semaphore_demo"
struct semaphore sem;
int major = 0 , minor = 0;

static int semaphore_open(struct inode *inode, struct file *file)
{
//int down_trylock(struct semaphore *sem)
	if(down_trylock(&sem))
	{
		printk("can not access...\n");
		return -EBUSY;
	}
	printk("%s,%d\n",__func__,__LINE__);

	return 0;
}
static int semaphore_close(struct inode *inode, struct file *file)
{
	//void up(struct semaphore *sem)
	up(&sem);

	printk("%s,%d\n",__func__,__LINE__);

	return 0;
}


static struct file_operations f_ops = {
	.owner = THIS_MODULE,
	.open = semaphore_open,
	.release = semaphore_close,
};
	
static int __init semaphore_init(void)
{
	major = register_chrdev(major,NAME,&f_ops);
	if(major < 0)
	{
		printk("register_chrdev fail ...%s,%d\n",__func__,__LINE__);
		return -EINVAL;
	}
	//static inline void sema_init(struct semaphore *sem, int val)
	sema_init(&sem , 2);

	printk("major :%d \t %s,%d\n",major ,__func__,__LINE__);

	return 0;
}

static void __exit semaphore_exit(void)
{
	unregister_chrdev(major,NAME);
	
	printk("%s,%d\n",__func__,__LINE__);

	return ;
}

module_init(semaphore_init);
module_exit(semaphore_exit);
