#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif

#define COUNT 			3
#define NAME 			"spinlock"
unsigned int major 		= 0;
unsigned int minor 		= 0;
struct cdev *cdev		= NULL;
int flags 				= 0;
spinlock_t lock; //������

static int spinlock_open(struct inode *inode, struct file *file)
{
	spin_lock(&lock);
	//spin_lock(&lock); //dead lock
	if(flags){
		spin_unlock(&lock);
		return -EBUSY;
	}
	flags++;
	spin_unlock(&lock);
	drv_pr("%s : %s : %d\n",__FILE__, __func__, __LINE__);

	return 0;
}

static ssize_t spinlock_read(struct file *file, char __user *ubuf, size_t size, loff_t * offs)
{
	drv_pr("%s : %s : %d\n",__FILE__, __func__, __LINE__);
	
	return 0;
}

static ssize_t spinlock_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offs)
{
	drv_pr("%s : %s : %d\n",__FILE__, __func__, __LINE__);

	return 0;
}

static int spinlock_close(struct inode *inode, struct file *file)
{
	drv_pr("%s : %s : %d\n",__FILE__, __func__, __LINE__);
	spin_lock(&lock);
	flags--;
	spin_unlock(&lock);

	return 0;
}

static struct file_operations fops = {
	.open    = spinlock_open,
	.read    = spinlock_read,
	.write   = spinlock_write,
	.release = spinlock_close,
};

static int __init spinlock_init(void)
{
	int ret;
	dev_t dev;
	cdev = cdev_alloc();
	if(cdev == NULL){
		drv_pr("alloc cdev error\n");
		ret = -ENOMEM;
		goto ERR_STP1;
	}
	
	cdev_init(cdev,&fops);

	if(major > 0){
		ret = register_chrdev_region(MKDEV(major,minor),COUNT,NAME);
		if(ret){
			drv_pr("static:alloc device num error\n");
			ret =  -EINVAL;
			goto ERR_STP1;
		}
	}else{
		ret = alloc_chrdev_region(&dev,0,COUNT,NAME);
		if(ret){
			drv_pr("Dynamic:alloc device number error");
			ret = -EINVAL;
			goto ERR_STP1;
		}
		major = MAJOR(dev);
		minor = MINOR(dev);

	}
	drv_pr("major = %d,minor = %d\n",major,minor);
	ret = cdev_add(cdev,MKDEV(major,minor),COUNT);
	if(ret){
		drv_pr("char device driver register error\n");
		ret = -EAGAIN;
		goto ERR_STP2;
	}
	//��ʼ����
	spin_lock_init(&lock);
	
	return 0;

ERR_STP2:
	unregister_chrdev_region(MKDEV(major,minor),COUNT);
ERR_STP1:
	return ret;
}

static void __exit spinlock_exit(void)
{
	cdev_del(cdev);
	unregister_chrdev_region(MKDEV(major,minor),COUNT);

	return ;
}

module_init(spinlock_init);
module_exit(spinlock_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("seedling");

