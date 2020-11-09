#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>


#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif

#define COUNT           3
#define NAME           "non_block_io"
unsigned int major      = 0;
unsigned int minor      = 0;
struct cdev *cdev       = NULL;

int condition           = 0;
char kbuf[128]          = { 0 };
wait_queue_head_t wq; //定义等待队列头

static int non_block_io_open(struct inode *inode, struct file *file)
{

	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

    return 0;
}

static ssize_t non_block_io_read(struct file *file, char __user *ubuf, size_t size, loff_t * offs)
{
	int ret;
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

    if(file->f_flags &  O_NONBLOCK){
		//非阻塞
		return -EINVAL;
	}else{
		//阻塞
		ret = wait_event_interruptible(wq,condition);
		if(ret){
			drv_pr("wait event interruptible error\n");
			return -EAGAIN;
		}
	}
	//拷贝数据
	if(size > sizeof(kbuf)) size = sizeof(kbuf);
	ret = copy_to_user(ubuf,kbuf,size);
	if(ret){
		drv_pr("copy data to user error\n");
		return -EINVAL;
	}
  //  0需要休眠，1不需要休眠
	//将条件设置为0
	condition = 0;

	return size;
}

static ssize_t non_block_io_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offs)
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

static int non_block_io_close(struct inode *inode, struct file *file)
{
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;
}

static struct file_operations fops = {
	.open    = non_block_io_open,
	.read    = non_block_io_read,
	.write   = non_block_io_write,
	.release = non_block_io_close,
};

static int __init non_block_io_init(void)
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
	//初始化等待队列头
	init_waitqueue_head(&wq);

	return 0;

ERR_STP2:
	unregister_chrdev_region(MKDEV(major,minor),COUNT);
ERR_STP1:
	return ret;
}

static void __exit non_block_io_exit(void)
{
	cdev_del(cdev);
	unregister_chrdev_region(MKDEV(major,minor),COUNT);

    return ;
}

module_init(non_block_io_init);
module_exit(non_block_io_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("seedling");

