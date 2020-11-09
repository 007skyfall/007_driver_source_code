#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#include <linux/wait.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");
 
#define MAX 4
struct resource *res1,*res2;
void __iomem *adccon;
int major = 0;
int count = 1;
struct class *cls  = NULL;
struct device *dev = NULL;

/*阻塞非阻塞IO相关   */
wait_queue_head_t wq;
int flags = 0 ;

#if 0

itop4412-adc{					
	compatible = "itop4412,adc";					
	reg = <0x126c0000 0x20>;					
	interrupt-parent = <&combiner>;					
	interrupts =<10 3>;				
};

/*arch/arm/boot/dts/exynos4412-itop-scp-core.dtsi*/

adc: adc@126C0000 {                                                                                                                                                                          
      compatible = "samsung,exynos-adc-v1";
      reg = <0x126C0000 0x100>;
      interrupt-parent = <&combiner>;
      interrupts = <10 3>;
      clocks = <&clock CLK_TSADC>;
      clock-names = "adc";
      #io-channel-cells = <1>;
      io-channel-ranges;
      samsung,syscon-phandle = <&pmu_system_controller>;
      status = "disabled"; 
    }; 

/*arch/arm/boot/dts/exynos4412-itop-elite.dts*/

&adc {
     /*vdd-supply = <&ldo3_reg>;*/
     status = "okay";                                                                                                                                                                             
   };          


#endif

#define ADCCON  0X0 
#define ADCDLY  0X8 
#define ADCDAT  0XC 
#define CLRINT  0X18
#define ADCMUX  0X1C

irqreturn_t adc_handler(int irq, void *data)
{
	writel(1,(adccon + CLRINT));//清除ADC的中断标志位
	flags = 1;
	wake_up(&wq);
//	printk("%s,%d\n",__func__,__LINE__);
	return IRQ_HANDLED;
}

static int adc_open(struct inode *inode, struct file *file)
{
	printk("%s,%d\n",__func__,__LINE__);
    
	return 0;
}

static int adc_release(struct inode *inode, struct file *file)
{
	printk("%s,%d\n",__func__,__LINE__);

	return 0;
}


static ssize_t adc_read(struct file *file, char __user *user, size_t size, loff_t *loff)
{
	int buf;
	writel(3,(adccon + ADCMUX));//ADC通道选择  3
	//writel(((1<<16)|(1<<14)|(255<<6)|(1<<1))&~(1<<2),adccon);//12bit|prescale en|div 255
	writel(((1<<16)|(1<<14)|(255<<6)|(1<<0)),adccon);//12bit|prescale en|div 255
	if(size > MAX)  
        size = MAX;
    
	if(!flags){
		if(file->f_flags & O_NONBLOCK)
			return -EAGAIN;
		if(wait_event_interruptible(wq,flags))
			return -ERESTARTSYS;
	}

	buf = readl(adccon + ADCDAT) & 0xfff;
	if(copy_to_user(user,&buf,sizeof(buf))){
		printk("copy_to_user fail...%s,%d\n",__func__,__LINE__);
	}

	flags = 0;
	printk("buf :%d \t%s,%d\n",buf,__func__,__LINE__);
    
	return size;
}

static struct file_operations f_ops = {
	.owner      = THIS_MODULE,
	.open       = adc_open,
	.release    = adc_release,
	.read       = adc_read,
};

static int pdrv_probe(struct platform_device *pdev)
{
	int ret = 0;
	/*完成对设备信息的采集，并根据设备信息来作出控制该设备的一系列逻辑代码*/
	/*1.获取设备树中相关的资源   reg  interrupt*/
	printk("%s,%d\n",__func__,__LINE__);

	res1 = platform_get_resource(pdev,IORESOURCE_MEM,0);
	if(res1 == NULL){
		printk("platform_get_resource ...%s,%d\n",__func__,__LINE__);
		return -ENOMEM;
	}

	res2 = platform_get_resource(pdev,IORESOURCE_IRQ,0);
	if(res2 == NULL){
		printk("platform_get_resource ...%s,%d\n",__func__,__LINE__);
		return -ENOMEM;
	}

	/*1.1  mem  相关操作*/
	adccon = ioremap(res1->start ,(res1->end - res1->start + 1));
	if(adccon == NULL){
		printk("ioremap ...%s,%d\n",__func__,__LINE__);
		return -ENOMEM;
	}

	/*1.2 irq 相关操作*/
	ret = request_irq(res2->start , adc_handler,IRQF_SHARED,"adc",NULL);
	if(ret< 0 ){
		printk("request_irq ...%s,%d\n",__func__,__LINE__);
		goto ERR_STEP1;
	}

	/*2.为了给用户空间提供一套操作本ADC的接口，所以在这创建一个char相关的操作*/
	major = register_chrdev(0,"adc",&f_ops);
	if(major < 0){
		printk("register_chrdev ...%s,%d\n",__func__,__LINE__);
		goto ERR_STEP2;
	}
	
	printk("%s,%d\n",__func__,__LINE__);
	/*2.1 自动创建设备节点 */
	cls = class_create(THIS_MODULE,"adc_class");
	if(IS_ERR(cls)){
		printk("class_create fail...%s,%d\n",__func__,__LINE__);
		ret = PTR_ERR(cls);
		goto ERR_STEP3;
	}
	dev = device_create(cls,NULL,MKDEV(major,0),NULL,"%s","adc");
	if(IS_ERR(dev)){
		printk("device_create fail...%s,%d\n",__func__,__LINE__);
		ret = PTR_ERR(dev);
		goto ERR_STEP4;
	}
	printk("%s,%d\n",__func__,__LINE__);
	
	init_waitqueue_head(&wq);//等待队列初始化:阻塞非阻塞IO的
	
	printk("major :%d %s,%d\n",major,__func__,__LINE__);

    return 0;
	
ERR_STEP4:
	class_destroy(cls);	
ERR_STEP3:
	unregister_chrdev(major,"adc");
ERR_STEP2:
	free_irq(res2->start,NULL);
ERR_STEP1:
	iounmap(adccon);
return ret;
}

static int pdrv_remove(struct platform_device *pdev)
{
	device_destroy(cls,MKDEV(major,0));
	class_destroy(cls);	
	unregister_chrdev(major,"adc");
	free_irq(res2->start,NULL);
	iounmap(adccon);
	printk("%s,%d\n",__func__,__LINE__);
    
	return 0;
}

const struct of_device_id	of_table[] = {
   [0] = {.compatible = "itop4412,adc"},
   [1] = {/*NULL*/},
};

struct platform_driver pdrv = {
	.probe = pdrv_probe,
	.remove = pdrv_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "xxxx",
		.of_match_table = of_match_ptr(of_table),//匹配设备树相关
	},
};

MODULE_DEVICE_TABLE(of,of_table);

module_platform_driver(pdrv);

