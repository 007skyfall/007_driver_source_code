#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/uaccess.h>

#define NAME "demo"
#define MAX  64
char ker_buf[MAX] = "i am from kernel...";
/*
1.ģ����Ҫ��
2.�ַ�������
	2.1 �����豸��
	2.2 cdev����
	2.3 cdev��ʼ��
	2.4 cdevע��
*/
ssize_t demo_read(struct file *file, char __user *user, size_t size, loff_t *loff)
{
	if(size > MAX)   size = MAX;
	
	if(copy_to_user(user,ker_buf,sizeof(ker_buf))){
		printk("copy_to_user fail...%s,%d\n",__func__,__LINE__);
		return -EAGAIN;
	}
	
	printk("%s,%d\n",__func__,__LINE__);
	return 0;
}

int demo_open (struct inode *inode, struct file *file)
{
	printk("%s,%d\n",__func__,__LINE__);
	return 0;
}
int demo_close (struct inode *inode, struct file *file)
{
	printk("%s,%d\n",__func__,__LINE__);
	return 0;
}

int major = 0 , minor = 0;

struct file_operations f_ops = {
	.owner = THIS_MODULE,
	.open = demo_open,
	.release = demo_close,
	.read = demo_read,
};
int __init demo_init(void)
{
	major = register_chrdev(major,NAME,&f_ops);
	if(major < 0){
		printk("register_chrdev fail ...%s,%d\n",__func__,__LINE__);
		return -EINVAL;
	}
	printk("major :%d \t %s,%d\n",major ,__func__,__LINE__);
	return 0;
}

void __exit demo_exit(void)
{
	unregister_chrdev(major,NAME);
	printk("%s,%d\n",__func__,__LINE__);
}

module_init(demo_init);
module_exit(demo_exit);
MODULE_LICENSE("GPL");










