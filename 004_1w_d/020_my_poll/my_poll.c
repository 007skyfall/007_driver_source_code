#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/poll.h>

MODULE_LICENSE("GPL");

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif

#define COUNT           3
#define NAME           "handle_block"
unsigned int major      = 0;
unsigned int minor      = 0;
struct cdev *cdev       = NULL;

int condition           = 0;
char kbuf[128]          = { 0 };
wait_queue_head_t wq; //1.定义等待队列头

static int my_poll_open(struct inode *inode, struct file *file)
{
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}

static ssize_t my_poll_read(struct file *file, char __user *ubuf, size_t size, loff_t * offs)
{
	int ret;

	//拷贝数据
	if(size > sizeof(kbuf)) size = sizeof(kbuf);
	ret = copy_to_user(ubuf,kbuf,size);
	if(ret){
		drv_pr("copy data to user error\n");
		return -EINVAL;
		
	}
	//将条件设置为0
	condition = 0;
	
	return size;
}

static ssize_t my_poll_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offs)
{
	int ret;
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

    if(size > sizeof(kbuf)) size = sizeof(kbuf);
	ret = copy_from_user(kbuf,ubuf,size);
	if(ret){
		drv_pr("copy data from user error\n");
		return -EINVAL;
	}
	condition = 1;
	wake_up_interruptible(&wq);

	return size;
}

static unsigned int my_poll_poll(struct file *file, struct poll_table_struct *wait)
{
	unsigned int mask = 0;

	poll_wait(file, &wq, wait);

	if(condition){
		mask |= POLLIN;
	}

	return mask;
}

static int my_poll_close(struct inode *inode, struct file *file)
{
    drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

    return 0;
}


static struct file_operations fops = {
	.open    = my_poll_open,
	.read    = my_poll_read,
	.write   = my_poll_write,
	.poll    = my_poll_poll,
	.release = my_poll_close,
};

static int __init my_poll_init(void)
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
	//2.初始化等待队列头
	init_waitqueue_head(&wq);

	return 0;

ERR_STP2:
	unregister_chrdev_region(MKDEV(major,minor),COUNT);
ERR_STP1:
	return ret;
}

static void __exit my_poll_exit(void)
{
	cdev_del(cdev);
	unregister_chrdev_region(MKDEV(major,minor),COUNT);

    return ;
}

module_init(my_poll_init);
module_exit(my_poll_exit);

MODULE_AUTHOR("seedling");

