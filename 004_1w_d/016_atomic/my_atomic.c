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

#define COUNT           3
#define NAME           "my_atomic"
unsigned int major      = 0;
unsigned int minor      = 0;
struct cdev *cdev       = NULL;

atomic_t lock = ATOMIC_INIT(-1);

static int my_atomic_open(struct inode *inode, struct file *file)
{
#if 0
/*
*	//减1之后和0比较，如果结果为0，表示获取锁成功了（返回值是真）
*   //否则获取锁失败了。
*/
	if(!atomic_dec_and_test(&lock)){
		atomic_inc(&lock);
		return -EBUSY;
	}
#else

    /*
    *   //加1之后和0比较，如果结果为0，表示获取锁成功了（返回值是真）
    *   //否则获取锁失败了。
    */
        if(!atomic_inc_and_test(&lock)){
            atomic_dec(&lock);
            return -EBUSY;
        }
#endif

    drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;
}

static ssize_t my_atomic_read(struct file *file, char __user *ubuf, size_t size, loff_t * offs)
{
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;
}

static ssize_t my_atomic_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offs)
{
    drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

    return 0;
}


static int my_atomic_close(struct inode *inode, struct file *file)
{
    drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);
#if 0
	atomic_inc(&lock);
#else
    atomic_dec(&lock);
#endif

	return 0;
}

static struct file_operations fops = {
	.open    = my_atomic_open,
	.read    = my_atomic_read,
	.write   = my_atomic_write,
	.release = my_atomic_close,
};

static int __init my_atomic_init(void)
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

	return 0;

ERR_STP2:
	unregister_chrdev_region(MKDEV(major,minor),COUNT);
ERR_STP1:
	return ret;
}

static void __exit my_atomic_exit(void)
{
	cdev_del(cdev);
	unregister_chrdev_region(MKDEV(major,minor),COUNT);

    return ;
}

module_init(my_atomic_init);
module_exit(my_atomic_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("seedling");

