/*
*	实现功能： HOME====LED2=====ON          BACK====LED2=====OFF
*	           V+====LED3=======ON     V-====LED3=======OFF	
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

MODULE_LICENSE("GPL");


//UART_RING========================GPX1_1=====HOME
//SIM_DET==========================GPX1_2=====BACK
//GYPO_INT=========================GPX3_3=====SLEEP
//KP_ROW1=========================GPX2_1======V+
//KP_ROW0=========================GPX2_0======V-

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif

#define GPX1  		    (0x11000C20)    /* HOME BACK */
#define GPX2  		    (0x11000C40)    /* V+   V- */

#define GPL2_CON		(0x11000100)    /* LED2 */
#define GPK1_CON		(0x11000060)    /* LED3 */

#define GPIO_SIZE  		        4

static struct class 	* interrupt_key_drv_class  = NULL;
static struct device	* interrupt_key_drv_device = NULL;

static struct class 	* interrupt_led_drv_class  = NULL;
static struct device	* interrupt_led_drv_device = NULL;

/* LED2 */
volatile unsigned long * gpl2con                   = NULL;
volatile unsigned long * gpl2dat                   = NULL;

/* LED3 */
volatile unsigned long * gpk1con                   = NULL;
volatile unsigned long * gpk1dat                   = NULL;

/* HOME BACK */
volatile unsigned long * gpx1con                   = NULL;
volatile unsigned long * gpx1dat                   = NULL;

/* V+ V- */
volatile unsigned long * gpx2con                   = NULL;
volatile unsigned long * gpx2dat                   = NULL;

int g_key_major = 0;
int g_led_major = 0;

static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* 中断事件标志, 中断服务程序将它置1，interrupt_key_led_drv_read将它清0 */
static volatile int ev_press = 0;

struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};

//
///* 键值: 按下时, 0x01, 0x02, 0x03, 0x04 */
///* 键值: 松开时, 0x81, 0x82, 0x83, 0x84 */

static unsigned char key_val;

struct pin_desc pins_desc[] = {
	{EXYNOS4_GPX1(1), 0x01},    /* HOME */
    {EXYNOS4_GPX1(2), 0x01},    /* BACK */    
	{EXYNOS4_GPX2(1), 0x01},    /*  V+  */
	{EXYNOS4_GPX2(0), 0x01},    /*  V-  */
};


/*
  * 确定按键值
  */
//static irqreturn_t led2_on_button_irq(int irq, void *dev_id)
//{
//	struct pin_desc * pindesc = (struct pin_desc *)dev_id;
//	unsigned int pinval;
//	
//	pinval = gpio_get_value(pindesc->pin);
//
//	if (pinval)
//	{
//		/* 松开 */
//		key_val = 0x80 | pindesc->key_val;
//		// LED2 OFF
//		*gpl2dat &= ~(0x01<<0);
//	}
//	else
//	{
//		/* 按下 */
//		key_val = pindesc->key_val;
//		// LED2 ON
//		*gpl2dat |= (0x01<<0);
//	}
//
//    ev_press = 1;                  /* 表示中断发生了 */
//    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */
//
//	
//	return IRQ_RETVAL(IRQ_HANDLED);
//}

static irqreturn_t led2_on_button_irq(int irq, void *dev_id)
{
//	struct pin_desc * pindesc = (struct pin_desc *)dev_id;
//	unsigned int pinval;
//	
//	pinval = gpio_get_value(pindesc->pin);
    *gpl2dat |= (0x01<<0);
//	if (pinval)
//	{
//		/* 松开 */
//		key_val = 0x80 | pindesc->key_val;
//    	// LED2 ON
//        *gpl2dat |= (0x01<<0);
//	}
//	else
//	{
//		/* 按下 */
//		key_val = pindesc->key_val;
//    	// LED2 ON
//        *gpl2dat |= (0x01<<0);
//	}

    ev_press = 1;                  /* 表示中断发生了 */
    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */

	
	return IRQ_RETVAL(IRQ_HANDLED);

}
#if 1
static irqreturn_t led2_off_button_irq(int irq, void *dev_id)
{
//	struct pin_desc * pindesc = (struct pin_desc *)dev_id;
//	unsigned int pinval;
//	
//	pinval = gpio_get_value(pindesc->pin);

	/* 按下 */
//	key_val = pindesc->key_val;
//	// LED2 OFF
	*gpl2dat &= ~(0x01<<0);
    
    ev_press = 1;                            /* 表示中断发生了 */
    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */
	
	return IRQ_RETVAL(IRQ_HANDLED);
}

//static irqreturn_t led3_button_irq(int irq, void *dev_id)
//{
//	struct pin_desc * pindesc = (struct pin_desc *)dev_id;
//	unsigned int pinval;
//	
//	pinval = gpio_get_value(pindesc->pin);
//
//	if (pinval)
//	{
//		/* 松开 */
//		key_val = 0x80 | pindesc->key_val;
//
//		// LED3熄灭
//		*gpk1dat &= ~(0x01<<1);
//	}
//	else
//	{
//		/* 按下 */
//		key_val = pindesc->key_val;
//
//		// LED3点灯
//		*gpk1dat |= (0x01<<1);
//	}
//
//    ev_press = 1;                  /* 表示中断发生了 */
//    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */
//
//	
//	return IRQ_RETVAL(IRQ_HANDLED);
//}

static irqreturn_t led3_on_button_irq(int irq, void *dev_id)
{
//	struct pin_desc * pindesc = (struct pin_desc *)dev_id;
//	unsigned int pinval;
//	
//	pinval = gpio_get_value(pindesc->pin);

	/* 按下 */
//	key_val = pindesc->key_val;
//	// LED3 ON
   *gpk1dat |= (0x01<<1);
    
    ev_press = 1;                           /* 表示中断发生了 */
    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */
	
	return IRQ_RETVAL(IRQ_HANDLED);
}

static irqreturn_t led3_off_button_irq(int irq, void *dev_id)
{
//	struct pin_desc * pindesc = (struct pin_desc *)dev_id;
//	unsigned int pinval;
//	
//	pinval = gpio_get_value(pindesc->pin);

	/* 按下 */
//	key_val = pindesc->key_val;
//	// LED3 OFF
   *gpk1dat &= ~(0x01<<1);

    ev_press = 1;                  /* 表示中断发生了 */
    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */
	
	return IRQ_RETVAL(IRQ_HANDLED);
}

#endif

static int interrupt_key_drv_open(struct inode *inode, struct file *file)
{
//	/* 配置GPL2_0为输出 */
	*gpl2con |= ~((0xf<<(4*0)))|(0x01<<(4*0));

//	/* 配置GPK1_1为输出 */
	*gpk1con |= ~((0xf<<(4*1)))|(0x01<<(4*1));

	/* 配置GPX1_1 GPX1_2为输入引脚        */
	request_irq(IRQ_EINT(9),  led2_on_button_irq,  IRQ_TYPE_EDGE_BOTH, "HOME",  &pins_desc[0]);
	request_irq(IRQ_EINT(10), led2_off_button_irq, IRQ_TYPE_EDGE_BOTH, "BACK",  &pins_desc[1]);

    /* 配置GPX2_0 GPX2_1为输入引脚 */
	request_irq(IRQ_EINT(17), led3_on_button_irq,   IRQ_TYPE_EDGE_BOTH,  "UP",    &pins_desc[2]);
	request_irq(IRQ_EINT(16), led3_off_button_irq,  IRQ_TYPE_EDGE_BOTH,  "DOWN",  &pins_desc[3]);
    
	return 0;
}

static ssize_t interrupt_key_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
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


static int interrupt_key_drv_close(struct inode *inode, struct file *file)
{
	free_irq(IRQ_EINT(9),   &pins_desc[0]);
	free_irq(IRQ_EINT(10),  &pins_desc[1]);
	free_irq(IRQ_EINT(17),  &pins_desc[2]);
	free_irq(IRQ_EINT(16),  &pins_desc[3]);
    
	return 0;
}

#if 0
static int interrupt_led_drv_open(struct inode *inode, struct file *file)
{
	drv_pr("------------itop_leds_drv_open------------\n");
    
	/* 配置GPL2_0为输出 */
	*gpl2con &= ~((0xf<<(4*0)))|(0x01<<(4*0));

	/* 配置GPK1_1为输出 */
	*gpk1con |= ~((0xf<<(4*1)))|(0x01<<(4*1));

    return 0;
}

static ssize_t interrupt_led_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val = 0;

	drv_pr("------------itop_leds_drv_write------------\n");
	
//    unsigned long copy_from_user(void *to, const void __user *from, unsigned long n)
	copy_from_user(&val, buf, count); //	copy_to_user();

    switch(val)
    {
        case 0:
        		// LED2点灯
    		    *gpl2dat |= (0x01<<0);  
                break;
        case 1:
                // LED2熄灭
                *gpl2dat &= ~(0x01<<0);
                break;
        case 2:
                // LED3点灯
                *gpk1dat |= (0x01<<1);
                break;
        case 3:
                // LED3熄灭
                *gpk1dat &= ~(0x01<<1);
                break;
        default:
                break;
    }

	return count;
}


static int interrupt_led_drv_close(struct inode *inode, struct file *file)
{
    // LED2熄灭
    *gpl2dat &= ~(0x01<<0);

    // LED3熄灭
    *gpk1dat &= ~(0x01<<1);

    return 0;
}

static struct file_operations interrupt_led_drv_fops = {
    .owner   =  THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open    =  interrupt_led_drv_open, 
    .write   =  interrupt_led_drv_write,
	.release =  interrupt_led_drv_close,	   
};

#endif

static struct file_operations interrupt_key_drv_fops = {
    .owner   =  THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open    =  interrupt_key_drv_open,     
	.read	 =	interrupt_key_drv_read,	   
	.release =  interrupt_key_drv_close,	   
};



static int __init interrupt_key_led_drv_init(void)
{
    int i = 1;
	g_key_major = register_chrdev(0, "interrupt_key_drv", &interrupt_key_drv_fops);

	interrupt_key_drv_class  = class_create(THIS_MODULE, "interrupt_key_drv");

    for(i=1; i<5; ++i){

        interrupt_key_drv_device = device_create(interrupt_key_drv_class, NULL, MKDEV(g_key_major, i),NULL, "button%d",i); /* /dev/button<1 2 3 4> */

    }
   #if 0 
	g_led_major = register_chrdev(0, "interrupt_led_drv", &interrupt_led_drv_fops);

	interrupt_led_drv_class  = class_create(THIS_MODULE, "interrupt_led_drv");

    for(i=2; i<4; ++i){

        interrupt_led_drv_device = device_create(interrupt_led_drv_class, NULL, MKDEV(g_led_major, i),NULL, "led%d",i); /* /dev/button<2 3> */
        
    }
#endif

	gpx1con = (volatile unsigned long *)ioremap(GPX1, GPIO_SIZE);
	gpx1dat = gpx1con + 1;

	gpx2con = (volatile unsigned long *)ioremap(GPX2, GPIO_SIZE);
	gpx2dat = gpx2con + 1;

	gpl2con = (volatile unsigned long *)ioremap(GPL2_CON, GPIO_SIZE);
	gpl2dat = gpl2con + 1;

	gpk1con = (volatile unsigned long *)ioremap(GPK1_CON, GPIO_SIZE);
	gpk1dat = gpk1con + 1;

	return 0;
}

static void __exit interrupt_key_led_drv_exit(void)
{
    int i = 0;
	unregister_chrdev(g_key_major, "interrupt_key_drv");

    for(i=1; i<5; ++i){

    device_destroy(interrupt_key_drv_class,MKDEV(g_key_major, i));

    }
	class_destroy(interrupt_key_drv_class);

	unregister_chrdev(g_led_major, "interrupt_key_drv");

//    for(i=2; i<4; ++i){
//
//    device_destroy(interrupt_led_drv_class,MKDEV(g_led_major, i));
//
//    }
	class_destroy(interrupt_led_drv_class);

	iounmap(gpx1con);
	iounmap(gpx2con);
	iounmap(gpl2con);
	iounmap(gpk1con);

	return ;
}

module_init(interrupt_key_led_drv_init);
module_exit(interrupt_key_led_drv_exit);

