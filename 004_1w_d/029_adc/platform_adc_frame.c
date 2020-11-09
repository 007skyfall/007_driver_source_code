#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");

struct resource *res[2];
int myresource_type[2] = {IORESOURCE_IO,IORESOURCE_IRQ};
//����ȴ�����ͷ

irqreturn_t handle_irq_handler_f(int irqno, void *dev)
{
	wake_up_interruptible(wq);
	condition = 1;

	return IRQ_HANDLED;
}

static int adc_open(struct inode *inode, struct file *file)
{	
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);	
	//1.��ʼ��adc�Ĵ���
	return 0;
}

static ssize_t adc_read(struct file *file, char __user *ubuf, size_t size, loff_t * offs)
{	
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	if(file->f_flags & O_NONBLOCK ){
		return -EAGAIN;
	}
	//1.����
	wait_event_interruptible(wq,condition);

	//2.��ȡadcdat�е�����
	//3.�����ݿ������û��ռ�
	if(size>sizeof(kbuf))
        size = sizeof(kbuf);
	ret = copy_to_user();

	condition = 0;
    
	return 0;
}

static int adc_close(struct inode *inode, struct file *file)
{	
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);	

    return 0;
}

struct file_operations fops = {
	.open = adc_open,
	.read = adc_read,
	.release = adc_close,
};


static int platform_drv_probe(struct platform_device *pdev)
{
	int i;
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
    
	for(i=0; i<2; i++){
		res[i] = platform_get_resource(pdev,
			myresource_type[i],0);
		if(res[i] == NULL){
			printk("get resource error\n");
			return -EAGAIN;
		}
	}
	printk("addr = %#x\n",res[0]->start);
	printk("irq = %d\n",res[1]->start);

	//1.ioreamp�������ַӳ��������ַ
	//2.request_irq(�жϴ�����)
	//3.ע���ַ��豸����
	//4.�Զ������豸�ڵ�
	//5.��ʼ���ȴ�����ͷ
	return 0;
}

static int platform_drv_remove(struct platform_device *pdev)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
    
	return 0;
}

static const struct of_device_id platform_pdrv_oftable[] = {
	{.compatible = "itop4412-adc",},
	{},
};	

struct platform_driver platform_drv = {
	.probe = platform_drv_probe,
	.remove = platform_drv_remove,
	.driver = {
		.name = "test",
		.owner = THIS_MODULE,
		.of_match_table = platform_pdrv_oftable,
	},
};

static int __init platform_drv_init(void)
{
	return platform_driver_register(&platform_drv);
}

static void __exit platform_drv_exit(void)
{
	platform_driver_unregister(&platform_drv);
}

module_init(platform_drv_init);
module_exit(platform_drv_exit);

