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
#define NAME           "handle_block"
unsigned int major      = 0;
unsigned int minor      = 0;
struct cdev *cdev       = NULL;

int condition = 0;
char kbuf[128] = {0};
wait_queue_head_t wq; //1.����ȴ�����ͷ
struct semaphore sem; //�����ź���

static int handle_block_open(struct inode *inode, struct file *file)
{
    drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

    return 0;
}

static ssize_t handle_block_read(struct file *file, char __user *ubuf, size_t size, loff_t * offs)
{
	int ret;
	//3.����ȴ�������
	DECLARE_WAITQUEUE(r_wait, current);

	//4.��ӵȴ�������
	add_wait_queue(&wq, &r_wait);
	
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	//5.�ж��Ƿ�����
	if(file->f_flags &  O_NONBLOCK){
		//������
		return -EINVAL;
	}
	if(condition == 0){
		if(down_trylock(&sem) != 0){
			drv_pr("try get lock error\n");
			ret = -EBUSY;
			goto ERR_STP1;
		}

		//6.���ý���״̬
			set_current_state(TASK_INTERRUPTIBLE);

			up(&sem);

		//7.����cpu
			schedule();
		
		if(down_trylock(&sem) != 0){
			drv_pr("try get lock error\n");
			ret = -EBUSY;
			goto ERR_STP1;
		}

		//8.�Ƿ��źŻ��ѵ�
		if (signal_pending(current)) {
				//�Ǳ��źŻ��ѵ�
			drv_pr("signal wake up\n");
			ret = -ERESTARTSYS;
			goto ERR_STP3;
		}

		//9.�л�����״̬
		set_current_state(TASK_RUNNING);

		up(&sem);
	}
	//10.��������
	if(size > sizeof(kbuf)) size = sizeof(kbuf);
	ret = copy_to_user(ubuf,kbuf,size);
	if(ret){
		drv_pr("copy data to user error\n");
		ret = -EINVAL;
		goto ERR_STP2;
	}
	//11.ɾ���ȴ�������
	remove_wait_queue(&wq, &r_wait);

	//12.����������Ϊ0
	condition = 0;
	
	return size;
	
ERR_STP3:
	up(&sem);
ERR_STP2:
	remove_wait_queue(&wq, &r_wait);
ERR_STP1:
	return ret;

}

static ssize_t handle_block_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offs)
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

static int handle_block_close(struct inode *inode, struct file *file)
{
    drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

    return 0;
}


static struct file_operations fops = {
	.open    = handle_block_open,
	.read    = handle_block_read,
	.write   = handle_block_write,
	.release = handle_block_close,
};

static int __init handle_block_init(void)
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
	//2.��ʼ���ȴ�����ͷ
	init_waitqueue_head(&wq);
	//��ʼ���ź���
	sema_init(&sem,1);
    
	return 0;

ERR_STP2:
	unregister_chrdev_region(MKDEV(major,minor),COUNT);
ERR_STP1:
	return ret;
}

static void __exit handle_block_exit(void)
{
	cdev_del(cdev);
	unregister_chrdev_region(MKDEV(major,minor),COUNT);

    return ;
}

module_init(handle_block_init);
module_exit(handle_block_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("seedling");

