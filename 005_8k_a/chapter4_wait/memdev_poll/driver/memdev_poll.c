/*********************************************************************************************
#####         上海嵌入式家园-开发板商城         #####
#####                    www.embedclub.com                        			#####
#####             http://embedclub.taobao.com               			#####

* File：		memdev_poll.c
* Author:		Hanson
* Desc：	Simple char device driver, non Block I/O
* History:	May 16th 2011
*********************************************************************************************/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/poll.h>

#include "memdev.h"

static mem_major = MEMDEV_MAJOR;

module_param(mem_major, int, S_IRUGO);

struct mem_dev *mem_devp; /*设备结构体指针*/

struct cdev cdev; 

bool have_data = false; // 判断设备是否有数据可读取标志

/*文件打开函数*/
int mem_open(struct inode *inode, struct file *filp)
{
    struct mem_dev *dev;
    
    /*获取次设备号*/
    int num = MINOR(inode->i_rdev);

    if (num >= MEMDEV_NR_DEVS) 
            return -ENODEV;
    dev = &mem_devp[num];
    
    /*将设备描述结构指针赋值给文件私有数据指针*/
    filp->private_data = dev;
    
    return 0; 
}

/*文件释放函数*/
int mem_release(struct inode *inode, struct file *filp)
{
  return 0;
}

/*读函数*/
static ssize_t mem_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
  unsigned long p =  *ppos;
  unsigned int count = size;
  int ret = 0;
  struct mem_dev *dev = filp->private_data; /*获得设备结构体指针*/

  printk("IN mem_read()\n");

  /*判断读位置是否有效*/
  if (p >= MEMDEV_SIZE)
    return 0;
  if (count > MEMDEV_SIZE - p)
    count = MEMDEV_SIZE - p;

  if(down_interruptible(&dev->sem))
  	return -ERESTARTSYS;

  while (!have_data) /*没有数据可读，考虑为什么不用if，而用while*/
  {
	up(&dev->sem);
	if (filp->f_flags & O_NONBLOCK)
		return -EAGAIN;
	wait_event_interruptible(dev->read_queue, have_data);

	if(down_interruptible(&dev->sem))
  		return -ERESTARTSYS;
  }

  /*读数据到用户空间*/
  if (copy_to_user(buf, (void*)(dev->data + p), count))
  {
    	ret =  - EFAULT;
	up(&dev->sem);
  } else {
    *ppos += count;
    ret = count;
    
    printk(KERN_INFO "read %d bytes(s) from %d\n", count, p);
  }

  have_data = false; /*标明不再有数据可读*/
  up(&dev->sem);
  /*唤醒写进程*/
  wake_up(&(dev->write_queue));
 
  return ret;
}

/*写函数*/
static ssize_t mem_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
  unsigned long p =  *ppos;
  unsigned int count = size;
  int ret = 0;
  struct mem_dev *dev = filp->private_data; /*获得设备结构体指针*/

  printk("IN mem_write()\n");
  
  /*分析和获取有效的写长度*/
  if (p >= MEMDEV_SIZE)
    return 0;
  if (count > MEMDEV_SIZE - p)
    count = MEMDEV_SIZE - p;

  if(down_interruptible(&dev->sem))
  	return -ERESTARTSYS;

  printk("IN mem_write() - 1\n");

   while (have_data) /*如果数据满*/
  {
	up(&dev->sem);
	if (filp->f_flags & O_NONBLOCK)
		return -EAGAIN;
	
	wait_event_interruptible(dev->write_queue, !have_data);

	if(down_interruptible(&dev->sem))
  		return -ERESTARTSYS;
  }
   printk("IN mem_write() - 2\n");
    
  /*从用户空间写入数据*/
  if (copy_from_user(dev->data + p, buf, count)) {
  	up(&dev->sem);
	ret =  - EFAULT;
  } else {
    *ppos += count;
    ret = count;
    
    printk("written %d bytes(s) from %d\n", count, p);
  }

  have_data = true; /*有新的数据可读*/
  up(&dev->sem);
  /*唤醒读进程*/
  wake_up(&(dev->read_queue));
  printk("out of mem_write()\n");

  return ret;
}

/* seek文件定位函数 */
static loff_t mem_llseek(struct file *filp, loff_t offset, int whence)
{ 
    loff_t newpos;

    switch(whence) {
      case 0: /* SEEK_SET */
        newpos = offset;
        break;

      case 1: /* SEEK_CUR */
        newpos = filp->f_pos + offset;
        break;

      case 2: /* SEEK_END */
        newpos = MEMDEV_SIZE -1 + offset;
        break;

      default: /* can't happen */
        return -EINVAL;
    }
    if ((newpos<0) || (newpos>MEMDEV_SIZE))
    	return -EINVAL;
    	
    filp->f_pos = newpos;
    return newpos;

}

static unsigned int mem_poll(struct file *filp, poll_table *wait)
{
	unsigned int mask = 0;
	struct mem_dev *dev = filp->private_data; /*获得设备结构体指针*/
  
	down(&dev->sem);
  
	poll_wait(filp, &dev->read_queue, wait);
	poll_wait(filp, &dev->write_queue, wait);
	
	/*memdev非空*/
	if (have_data)
  	{
    		mask |= POLLIN | POLLRDNORM; /*表示数据可获得*/
		printk("In mem_poll, have_data is true\n");
 	 }
  	/*memdev非满*/
  	if (!have_data)
	{
		mask |= POLLOUT | POLLWRNORM; /*表示数据可写入*/
		printk("In mem_poll, have_data is false\n");
	}
     
	up(&dev->sem);
	return mask;
}

/*文件操作结构体*/
static const struct file_operations mem_fops =
{
  .owner = THIS_MODULE,
  .llseek = mem_llseek,
  .poll = mem_poll,
  .read = mem_read,
  .write = mem_write,
  .open = mem_open,
  .release = mem_release,
};

/*设备驱动模块加载函数*/
static int memdev_init(void)
{
  int result;
  int i;
  char dev_name[] = "memdev";

  dev_t devno = MKDEV(mem_major, 0);

  /* 静态申请设备号*/
  if (mem_major)
    result = register_chrdev_region(devno, 2, dev_name);
  else  /* 动态分配设备号 */
  {
    result = alloc_chrdev_region(&devno, 0, 2, dev_name);
    mem_major = MAJOR(devno);
  }  
  
  if (result < 0)
    return result;

  /*初始化cdev结构*/
  cdev_init(&cdev, &mem_fops);
  cdev.owner = THIS_MODULE;
  cdev.ops = &mem_fops;
  
  /* 注册字符设备 */
  cdev_add(&cdev, MKDEV(mem_major, 0), MEMDEV_NR_DEVS);
   
  /* 为设备描述结构分配内存*/
  mem_devp = kmalloc(MEMDEV_NR_DEVS * sizeof(struct mem_dev), GFP_KERNEL);
  if (!mem_devp)    /*申请失败*/
  {
    result =  - ENOMEM;
    goto fail_malloc;
  }
  memset(mem_devp, 0, sizeof(struct mem_dev));
  
  /*为设备分配内存*/
  for (i=0; i < MEMDEV_NR_DEVS; i++) 
  {
        mem_devp[i].size = MEMDEV_SIZE;
        mem_devp[i].data = kmalloc(MEMDEV_SIZE, GFP_KERNEL);
        memset(mem_devp[i].data, 0, MEMDEV_SIZE);
	init_waitqueue_head(&mem_devp[i].read_queue);
	init_waitqueue_head(&mem_devp[i].write_queue);
	init_MUTEX(&mem_devp[i].sem);
	printk("The device name is /dev/%s%d\n", dev_name, i);
  }

  printk("mem device installed, with major %d\n", mem_major);
    
  return 0;

  fail_malloc: 
  unregister_chrdev_region(devno, 1);
  
  return result;
}

/*模块卸载函数*/
static void memdev_exit(void)
{
  cdev_del(&cdev);   /*注销设备*/
  kfree(mem_devp);     /*释放设备结构体内存*/
  unregister_chrdev_region(MKDEV(mem_major, 0), 2); /*释放设备号*/
}

MODULE_AUTHOR("Hanson He");
MODULE_LICENSE("GPL");

module_init(memdev_init);
module_exit(memdev_exit);
