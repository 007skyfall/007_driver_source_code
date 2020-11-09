#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/types.h>

MODULE_LICENSE("GPL");

#define NAME "atomic_demo"
int flag = 0;
atomic_t  at = ATOMIC_INIT(1);

static  int demo_open(struct inode *inode, struct file *file)
{
//static __inline__ int atomic_dec_and_test(atomic_t *v)
//static inline void atomic_inc(atomic_t *v)
	if(!atomic_dec_and_test(&at))
	{
		atomic_inc(&at);
		return -EBUSY;
	}
	printk("%s,%d\n",__func__,__LINE__);
	return 0;
}

static int demo_close(struct inode *inode, struct file *file)
{
	atomic_dec(&at);
	printk("%s,%d\n",__func__,__LINE__);
	return 0;
}

int major = 0 , minor = 0;

struct file_operations f_ops = {
	.owner = THIS_MODULE,
	.open = demo_open,
	.release = demo_close,
};
	
static int __init demo_init(void)
{
	major = register_chrdev(major,NAME,&f_ops);
	if(major < 0)
	{
		printk("register_chrdev fail ...%s,%d\n",__func__,__LINE__);
		return -EINVAL;
	}
	printk("major :%d \t %s,%d\n",major ,__func__,__LINE__);
	return 0;
}

static void __exit demo_exit(void)
{
	unregister_chrdev(major,NAME);
	printk("%s,%d\n",__func__,__LINE__);

	return ;
}

module_init(demo_init);
module_exit(demo_exit);
