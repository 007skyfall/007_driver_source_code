#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#define CON 0x11000c20
#define DAT 0x11000c24
#define COUNT 3
#define NAME "mycdev"
unsigned int major = 0;
unsigned int minor = 0;
unsigned int *con;
unsigned int *dat;
struct cdev *cdev;
char kbuf[128] = {0};

int mycdev_open(struct inode *inode, struct file *file)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}

ssize_t mycdev_read(struct file *file, char __user *ubuf, size_t size, loff_t * offs)
{
	int ret;
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	if(size >= sizeof(kbuf)) size = sizeof(kbuf);
	ret = copy_to_user(ubuf,kbuf,size);
	if(ret){
		printk("copy data to user error\n");
		return -EINVAL;
	}
	
	return size;
}

ssize_t mycdev_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offs)
{
	int ret;
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	if(size > sizeof(kbuf)) size = sizeof(kbuf);
	ret = copy_from_user(kbuf,ubuf,size);
	if(ret){
		printk("copy data from user error\n");
		return -EINVAL;
	}
	if(kbuf[0] == 1){
		if(kbuf[1] == 1){
			*dat |= (0x1<<0);
		}else if(kbuf[1] == 0){
			*dat &= ~(0x1<<0);
		}
	}
	return size;
}

int mycdev_close(struct inode *inode, struct file *file)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}

static struct file_operations fops = {
	.open    = mycdev_open,
	.read    = mycdev_read,
	.write   = mycdev_write,
	.release = mycdev_close,
};

static int __init mycdev_init(void)
{
	int ret;
	dev_t dev;
	cdev = cdev_alloc();
	if(cdev == NULL){
		printk("alloc cdev error\n");
		ret = -ENOMEM;
		goto ERR_STP1;
	}
	
	cdev_init(cdev,&fops);

	if(major > 0){
		ret = register_chrdev_region(MKDEV(major,minor),COUNT,NAME);
		if(ret){
			printk("static:alloc device num error\n");
			ret =  -EINVAL;
			goto ERR_STP1;
		}
	}else{
		ret = alloc_chrdev_region(&dev,0,COUNT,NAME);
		if(ret){
			printk("Dynamic:alloc device number error");
			ret = -EINVAL;
			goto ERR_STP1;
		}
		major = MAJOR(dev);
		minor = MINOR(dev);

	}
	printk("major = %d,minor = %d\n",major,minor);
	ret = cdev_add(cdev,MKDEV(major,minor),COUNT);
	if(ret){
		printk("char device driver register error\n");
		ret = -EAGAIN;
		goto ERR_STP2;
	}

	con = ioremap(CON,4);
	if(con == NULL){
		printk("ioremap con register error\n");
		ret =  -ENOMEM;
		goto ERR_STP3;
	}
	
	dat = ioremap(DAT,4);
	if(dat == NULL){
		printk("ioremap dat register error\n");
		ret =  -ENOMEM;
		goto ERR_STP4;
	}
	*con &= ~(0xf<<0);
	*con |= (0x1<<0);
	*dat &= ~(0x1<<0);
	
	return 0;
ERR_STP4:
	iounmap(con);
ERR_STP3:
	cdev_del(cdev);
ERR_STP2:
	unregister_chrdev_region(MKDEV(major,minor),COUNT);
ERR_STP1:
	return ret;
}

static void __exit mycdev_exit(void)
{
	iounmap(dat);
	iounmap(con);
	cdev_del(cdev);
	unregister_chrdev_region(MKDEV(major,minor),COUNT);
}

module_init(mycdev_init);
module_exit(mycdev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("farsight daizs_ck@hqyj.com");
