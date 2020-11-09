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

#define COUNT				 3
#define NAME 				"semaphore"
unsigned int major 			 = 0;
unsigned int minor 			 = 0;
struct cdev *cdev			 = NULL;
struct semaphore sem;

static int semaphore_open(struct inode *inode, struct file *file)
{
	/*第一个进程打开后，后面的进程再次打开down_trylock的返回值不是0，所以
	* 进入到if语句，返回busy。
	*/
	if(down_trylock(&sem) != 0){
		return -EBUSY;
	}

	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	
	return 0;
}

static ssize_t semaphore_read(struct file *file, char __user *ubuf, size_t size, loff_t * offs)
{
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;
}

static ssize_t semaphore_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offs)
{
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;
}

static int semaphore_close(struct inode *inode, struct file *file)
{
	drv_pr("%s : %s : %d\n",__FILE__, __func__, __LINE__);

	up(&sem);

	return 0;
}

static struct file_operations fops = {
	.open    = semaphore_open,
	.read    = semaphore_read,
	.write   = semaphore_write,
	.release = semaphore_close,
};

static int __init semaphore_init(void)
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
	//初始化锁
	sema_init(&sem,1);
	
	return 0;

ERR_STP2:
	unregister_chrdev_region(MKDEV(major,minor),COUNT);
ERR_STP1:
	return ret;
}

static void __exit semaphore_exit(void)
{
	cdev_del(cdev);
	unregister_chrdev_region(MKDEV(major,minor),COUNT);

	return ;
}

module_init(semaphore_init);
module_exit(semaphore_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("seedling");

