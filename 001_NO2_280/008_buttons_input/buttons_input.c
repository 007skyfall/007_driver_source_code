
/* 参考drivers/input/keyboard/gpio_keys.c */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <mach/gpio-exynos4.h>
#include <linux/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <mach/regs-gpio.h>
#include <asm/io.h>

//UART_RING==GPX1_1==HOME==IRQ_EINT(9)
//SIM_DET====GPX1_2==BACK==IRQ_EINT(10)
//GYPO_INT===GPX3_3==SLEEP==IRQ_EINT(27)
//KP_ROW1====GPX2_1==UP====IRQ_EINT(17)
//KP_ROW0====GPX2_0==SOWN==IRQ_EINT(16)

MODULE_LICENSE("GPL");

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif

#define	ITOP_KEY_HOME		EXYNOS4_GPX1(1)
#define	ITOP_KEY_BACK		EXYNOS4_GPX1(2)
#define	ITOP_KEY_SLEEP		EXYNOS4_GPX3(3)
#define ITOP_KEY_VOL_U		EXYNOS4_GPX2(1)
#define ITOP_KEY_VOL_D		EXYNOS4_GPX2(0)

struct pin_desc{
	int irq;
	char *name;
	unsigned int pin;
	unsigned int key_val;
};


struct pin_desc pins_desc[4] = {
	{IRQ_EINT(9),   "HOME",  ITOP_KEY_HOME,   KEY_L},
	{IRQ_EINT(10),  "BACK",  ITOP_KEY_BACK,   KEY_S},
    {IRQ_EINT(17),  "UP",    ITOP_KEY_VOL_U,  KEY_ENTER},
	{IRQ_EINT(16),  "DOWN",  ITOP_KEY_VOL_D,  KEY_LEFTSHIFT},

};

static struct input_dev *buttons_dev;
static struct pin_desc *irq_pd;
static struct timer_list buttons_timer;

static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	/* 10ms后启动定时器 */
	irq_pd = (struct pin_desc *)dev_id;
	mod_timer(&buttons_timer, jiffies+HZ/100);
	return IRQ_RETVAL(IRQ_HANDLED);
}

static void buttons_timer_function(unsigned long data)
{
	struct pin_desc * pindesc = irq_pd;
	unsigned int pinval;

	if (!pindesc)
		return;
	
	pinval = gpio_get_value(pindesc->pin);

	if (pinval)
	{
		/* 松开 : 最后一个参数: 0-松开, 1-按下 */
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 0);
		input_sync(buttons_dev);
	}
	else
	{
		/* 按下 */
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 1);
		input_sync(buttons_dev);
	}
}

static int __init buttons_init(void)
{
	int i;
	
	/* 1. 分配一个input_dev结构体 */
	buttons_dev = input_allocate_device();

	/* 2. 设置 */
	/* 2.1 能产生哪类事件 */
	set_bit(EV_KEY, buttons_dev->evbit);
	set_bit(EV_REP, buttons_dev->evbit);
	
	/* 2.2 能产生这类操作里的哪些事件: L,S,ENTER,LEFTSHIT */
	set_bit(KEY_L, buttons_dev->keybit);
	set_bit(KEY_S, buttons_dev->keybit);
	set_bit(KEY_ENTER, buttons_dev->keybit);
	set_bit(KEY_LEFTSHIFT, buttons_dev->keybit);

	/* 3. 注册 */
	input_register_device(buttons_dev);
	
	/* 4. 硬件相关的操作 */
	init_timer(&buttons_timer);
	buttons_timer.function = buttons_timer_function;/*定时器处理函数*/
	add_timer(&buttons_timer);

	for (i = 0; i < 4; i++)
	{
		request_irq(pins_desc[i].irq, buttons_irq, IRQ_TYPE_EDGE_BOTH, pins_desc[i].name, &pins_desc[i]);
	}
	
	return 0;
}

static void __exit buttons_exit(void)
{
	int i;
	
	for (i = 0; i < 4; i++)
	{
		free_irq(pins_desc[i].irq, &pins_desc[i]);
	}

	del_timer(&buttons_timer);
	input_unregister_device(buttons_dev);
	input_free_device(buttons_dev);	

	return ;
}

module_init(buttons_init);

module_exit(buttons_exit);

