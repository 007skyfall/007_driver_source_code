#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/gpio.h>
#include <mach/gpio-exynos4.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <mach/regs-gpio.h>
#include <asm/io.h>
#include <linux/poll.h>



MODULE_LICENSE("GPL");

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif

/*
*   UART_RING========================GPX1_1=====HOME
*   SIM_DET==========================GPX1_2=====BACK
*   GYPO_INT=========================GPX3_3=====SLEEP
*   KP_ROW1=========================GPX2_1=======V+
*   KP_ROW0=========================GPX2_0=======V-
*
*/
#define GPX1  	        	(0x11000C20)
#define GPX2  		        (0x11000C40)
#define GPX3  		        (0x11000C60)
#define GPIO_SIZE  	        	8

volatile unsigned long  *gpx1con            = NULL;
volatile unsigned long  *gpx1dat            = NULL;       
volatile unsigned long  *gpx2con            = NULL;
volatile unsigned long  *gpx2dat            = NULL;
volatile unsigned long  *gpx3con            = NULL;
volatile unsigned long  *gpx3dat            = NULL;

static struct class     *buttons_class  = NULL;
static struct device	*buttons_device = NULL;

static struct timer_list buttons_timer;

static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* 中断事件标志, 中断服务程序将它置1，buttons_read将它清0 */
static volatile int ev_press = 0;

static struct fasync_struct *button_async;
int major;

struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};


/* 键值: 按下时, 0x01, 0x02, 0x03, 0x04 */
/* 键值: 松开时, 0x81, 0x82, 0x83, 0x84 */
static unsigned char key_val;

struct pin_desc pins_desc[] = {
	{EXYNOS4_GPX1(1), 0x01},    /* HOME */
    {EXYNOS4_GPX1(2), 0x01},    /* BACK */    
	{EXYNOS4_GPX2(1), 0x01},    /*  V+  */
	{EXYNOS4_GPX2(0), 0x01},    /*  V-  */
};


static struct pin_desc *irq_pd;

//static atomic_t canopen = ATOMIC_INIT(1);     //定义原子变量并初始化为1

static DEFINE_SEMAPHORE(button_lock);     //定义互斥锁

/*
  * 确定按键值
  */
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	/* 10ms后启动定时器 */
	irq_pd = (struct pin_desc *)dev_id;
	mod_timer(&buttons_timer, jiffies+HZ/100);
	return IRQ_RETVAL(IRQ_HANDLED);
}

static int buttons_open(struct inode *inode, struct file *file)
{
#if 0	
	if (!atomic_dec_and_test(&canopen))
	{
		atomic_inc(&canopen);
		return -EBUSY;
	}
#endif		

	if (file->f_flags & O_NONBLOCK)
	{
		if (down_trylock(&button_lock))
			return -EBUSY;
	}
	else
	{
		/* 获取信号量 */
		down(&button_lock);
	}
    
	/* 配置GPX1_1,GPX1_2为输入引脚 */
	*gpx1con &= ~((0xf<<(1*4)) | (0x0<<(1*4)) | (0xf<<(2*4)) | (0x0<<(2*4)));
    
	request_irq(IRQ_EINT(9),  buttons_irq,  IRQ_TYPE_EDGE_BOTH, "HOME",  &pins_desc[0]);
	request_irq(IRQ_EINT(10), buttons_irq,  IRQ_TYPE_EDGE_BOTH, "BACK",  &pins_desc[1]);

	
	/* 配置GPX2_0,GPX2_1为输入引脚 */
	*gpx2con &= ~((0xf<<(0*4)))|(0x0<<(0*4) | (0xf<<(1*4)))|(0x0<<(1*4));
	request_irq(IRQ_EINT(17), buttons_irq,   IRQ_TYPE_EDGE_BOTH,  "UP",    &pins_desc[2]);
	request_irq(IRQ_EINT(16), buttons_irq,   IRQ_TYPE_EDGE_BOTH,  "DOWN",  &pins_desc[3]);

	return 0;
}

static ssize_t buttons_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	if (size != 1)
		return -EINVAL;

	if (file->f_flags & O_NONBLOCK)
	{
		if (!ev_press)
			return -EAGAIN;
	}
	else
	{
		/* 如果没有按键动作, 休眠 */
		wait_event_interruptible(button_waitq, ev_press);
	}

	/* 如果有按键动作, 返回键值 */
	copy_to_user(buf, &key_val, 1);
	ev_press = 0;
	
	return 1;
}


static int buttons_close(struct inode *inode, struct file *file)
{
	//atomic_inc(&canopen);
	free_irq(IRQ_EINT(9),   &pins_desc[0]);
	free_irq(IRQ_EINT(10),  &pins_desc[1]);
	free_irq(IRQ_EINT(16),  &pins_desc[2]);
	free_irq(IRQ_EINT(17),  &pins_desc[3]);
    
	up(&button_lock);
    
	return 0;
}

static unsigned int buttons_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	poll_wait(file, &button_waitq, wait); // 不会立即休眠
	if (ev_press)
		mask |= POLLIN | POLLRDNORM;

	return mask;
}

static int buttons_fasync (int fd, struct file *filp, int on)
{
	drv_pr("driver: buttons_fasync\n");
    
	return fasync_helper (fd, filp, on, &button_async);
}


static struct file_operations buttons_drv_fops = {
    .owner   =  THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open    =  buttons_open,     
	.read	 =	buttons_read,	   
	.release =  buttons_close,
	.poll    =  buttons_poll,
	.fasync	 =  buttons_fasync,
};

static void buttons_timer_function(unsigned long data)
{
	struct pin_desc * pindesc = irq_pd;
	unsigned int pinval;

	if (!pindesc)
		return;
	pinval = gpio_get_value(pindesc->pin);

	if (pinval)
	{
		/* 松开 */
		key_val = 0x80 | pindesc->key_val;
	}
	else
	{
		/* 按下 */
		key_val = pindesc->key_val;
	}

    ev_press = 1;                  /* 表示中断发生了 */
    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */
	
	kill_fasync (&button_async, SIGIO, POLL_IN);
}

static int __init buttons_init(void)
{
    drv_pr("%s : %s : %d\n",__FILE__, __func__, __LINE__);
    
    init_timer(&buttons_timer);
	buttons_timer.function = buttons_timer_function;
	//buttons_timer.expires  = 0;
	add_timer(&buttons_timer); 

	major = register_chrdev(0, "buttons", &buttons_drv_fops);

	buttons_class = class_create(THIS_MODULE, "buttons");

	/* 为了让mdev根据这些信息来创建设备节点 */
	buttons_device = device_create(buttons_class, NULL, MKDEV(major, 0), NULL, "buttons"); /* /dev/buttons */

	gpx1con = (volatile unsigned long *)ioremap(GPX1, GPIO_SIZE);
	gpx1dat = gpx1con + 1;

	gpx2con = (volatile unsigned long *)ioremap(GPX2, GPIO_SIZE);
	gpx2dat = gpx2con + 1;

	return 0;
}

static void __exit buttons_exit(void)
{
	unregister_chrdev(major, "buttons");
	device_destroy(buttons_class,MKDEV(major, 0));
	class_destroy(buttons_class);
	iounmap(gpx1con);
	iounmap(gpx2con);
    
	return ;
}

module_init(buttons_init);
module_exit(buttons_exit);

