/*实现功能：   使用poll机制实现。
			 使用HOME按键控制LED2灯，当按下时，LED2熄灭，当松开时LED2点亮。
*		     使用VOL-按键控制LED3灯，当按下时，LED3点亮，当松开时LED3熄灭。
*			 使用SLEEP按键控制LED灯，当按下时，全部点亮，当松开时全部熄灭。	
*/

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



#define GPX1  		(0x11000C20)
#define GPX2  		(0x11000C40)
#define GPX3  		(0x11000C60)
#define GPL2_CON	(0x11000100)
#define GPK1_CON	(0x11000060)
#define GPIO_SIZE  		8

MODULE_LICENSE("GPL");

static struct class 	*fasync_key_led_class;
static struct device	*fasync_key_led_device;

volatile unsigned long 	*gpx1con = NULL;
volatile unsigned long 	*gpx1dat = NULL;

volatile unsigned long	*gpx2con = NULL;
volatile unsigned long  *gpx2dat = NULL;

volatile unsigned long  *gpx3con = NULL;
volatile unsigned long  *gpx3dat = NULL;

volatile unsigned long  *gpl2con = NULL;
volatile unsigned long  *gpl2dat = NULL;

volatile unsigned long  *gpk1con = NULL;
volatile unsigned long  *gpk1dat = NULL;


static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* 中断事件标志, 中断服务程序将它置1，fifth_drv_read将它清0 */
static volatile int ev_press = 0;

static struct fasync_struct *button_async;


struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};


/* 键值: 按下时, 0x01, 0x02, 0x03 */
/* 键值: 松开时, 0x81, 0x82, 0x83 */
static unsigned char key_val;
int major;

struct pin_desc pins_desc[3] = {
	{EXYNOS4_GPX1(1), 0x01},
	{EXYNOS4_GPX2(0), 0x02},
	{EXYNOS4_GPX3(3), 0x03},
};

/*
  * 确定按键值
  */
static irqreturn_t home_button_irq(int irq, void *dev_id)
{
	struct pin_desc * pindesc = (struct pin_desc *)dev_id;
	unsigned int pinval;
	
	pinval = gpio_get_value(pindesc->pin);

	if (pinval)
	{
		/* 松开 */
		key_val = 0x80 | pindesc->key_val;
		// LED2点亮
		*gpl2dat |= (0x01<<0);
	}
	else
	{
		/* 按下 */
		key_val = pindesc->key_val;
		// LED2灭灯
		*gpl2dat &= ~(0x01<<0);
	}

    ev_press = 1;                  /* 表示中断发生了 */
    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */

	
	return IRQ_RETVAL(IRQ_HANDLED);
}

static irqreturn_t vol_button_irq(int irq, void *dev_id)
{
	struct pin_desc * pindesc = (struct pin_desc *)dev_id;
	unsigned int pinval;
	
	pinval = gpio_get_value(pindesc->pin);

	if (pinval)
	{
		/* 松开 */
		key_val = 0x80 | pindesc->key_val;

		// LED3熄灭
		*gpk1dat &= ~(0x01<<1);
	}
	else
	{
		/* 按下 */
		key_val = pindesc->key_val;

		// LED3点灯
		*gpk1dat |= (0x01<<1);
	}

    ev_press = 1;                  /* 表示中断发生了 */
    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */

	
	return IRQ_RETVAL(IRQ_HANDLED);
}

static irqreturn_t sleep_button_irq(int irq, void *dev_id)
{
	struct pin_desc * pindesc = (struct pin_desc *)dev_id;
	unsigned int pinval;
	
	pinval = gpio_get_value(pindesc->pin);

	if (pinval)
	{
		/* 松开 */
		key_val = 0x80 | pindesc->key_val;

		// LED2点亮
		*gpl2dat |= (0x01<<0);
		// LED3点灯
		*gpk1dat |= (0x01<<1);
	}
	else
	{
		/* 按下 */
		key_val = pindesc->key_val;

		// LED2熄灭
		*gpl2dat &= ~(0x01<<0);
		// LED3熄灭
		*gpk1dat &= ~(0x01<<1);
	}

    ev_press = 1;                  /* 表示中断发生了 */
    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */

	
	return IRQ_RETVAL(IRQ_HANDLED);
}

static int fasync_key_led_open(struct inode *inode, struct file *file)
{
	/* 配置GPL2_0为输出 */
	*gpl2con &= ~((0xf<<(4*0)))|(0x01<<(4*0));

	/* 配置GPK1_1为输出 */
	*gpk1con |= ~((0xf<<(4*1)))|(0x01<<(4*1));

	/* 配置GPX1_1为输入引脚 */
	/* 配置GPX2_0为输入引脚 */
	/* 配置GPX3_3为输入引脚 */

	request_irq(IRQ_EINT(9),home_button_irq,IRQ_TYPE_EDGE_BOTH,"HOME",&pins_desc[0]);
	request_irq(IRQ_EINT(16),vol_button_irq,IRQ_TYPE_EDGE_BOTH,"VOL-",&pins_desc[1]);	
	request_irq(IRQ_EINT(27),sleep_button_irq,IRQ_TYPE_EDGE_BOTH,"SLEEP",&pins_desc[2]);	

	return 0;
}

ssize_t fasync_key_led_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	if (size != 1)
		return -EINVAL;

	/* 如果没有按键动作, 休眠 */
	wait_event_interruptible(button_waitq, ev_press);

	/* 如果有按键动作, 返回键值 */
	copy_to_user(buf, &key_val, 1);
	ev_press = 0;
	
	return 1;
}


int fasync_key_led_close(struct inode *inode, struct file *file)
{
	free_irq(IRQ_EINT(9),  &pins_desc[0]);
	free_irq(IRQ_EINT(16), &pins_desc[1]);
	free_irq(IRQ_EINT(27), &pins_desc[2]);

	return 0;
}

static unsigned fasync_key_led_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	poll_wait(file, &button_waitq, wait); // 不会立即休眠

	if (ev_press)
		mask |= POLLIN | POLLRDNORM;

	return mask;
}

static int fasync_key_led_fasync (int fd, struct file *filp, int on)
{
	printk("driver: fasync_key_led_fasync!\n");
	return fasync_helper (fd, filp, on, &button_async);
}


static struct file_operations fasync_key_led_fops = {
    .owner   =  THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open    =  fasync_key_led_open,     
	.read	 =	fasync_key_led_read,	   
	.release =  fasync_key_led_close,
	.poll    =  fasync_key_led_poll,
	.fasync	 =  fasync_key_led_fasync,
};

static int __init fasync_key_led_init(void)
{
	major = register_chrdev(0, "fasync_key_led_drv", &fasync_key_led_fops);

	fasync_key_led_class = class_create(THIS_MODULE, "fasync_key_led_drv");
	fasync_key_led_device = device_create(fasync_key_led_class, NULL, 
							MKDEV(major, 0), NULL, "button"); /* /dev/button */

	gpx1con = (volatile unsigned long *)ioremap(GPX1, GPIO_SIZE);
	gpx1dat = gpx1con + 1;

	gpx2con = (volatile unsigned long *)ioremap(GPX2, GPIO_SIZE);
	gpx2dat = gpx2con + 1;

	gpx3con = (volatile unsigned long *)ioremap(GPX3, GPIO_SIZE);
	gpx3dat = gpx3con + 1;

	gpl2con = (volatile unsigned long *)ioremap(GPL2_CON, GPIO_SIZE);
	gpl2dat = gpl2con + 1;

	gpk1con = (volatile unsigned long *)ioremap(GPK1_CON, GPIO_SIZE);
	gpk1dat = gpk1con + 1;

	return 0;
}

static void __exit fasync_key_led_exit(void)
{
	unregister_chrdev(major, "fasync_key_led_drv");

	device_destroy(fasync_key_led_class,MKDEV(major, 0));
	class_destroy(fasync_key_led_class);

	iounmap(gpx1con);
	iounmap(gpx2con);
	iounmap(gpx3con);
	iounmap(gpl2con);
	iounmap(gpk1con);

	return ;
}

module_init(fasync_key_led_init);
module_exit(fasync_key_led_exit);