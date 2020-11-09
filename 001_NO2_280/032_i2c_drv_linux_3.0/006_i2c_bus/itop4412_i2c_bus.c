#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of_i2c.h>
#include <linux/of_gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/regs-gpio.h>
#include <asm/irq.h>
#include <plat/regs-iic.h>
#include <plat/iic.h>

MODULE_LICENSE("GPL");

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif
enum itop4412_i2c_state {
	STATE_IDLE,
	STATE_START,
	STATE_READ,
	STATE_WRITE,
	STATE_STOP
};

struct itop4412_i2c_regs {
	unsigned int iiccon;
	unsigned int iicstat;
	unsigned int iicadd;
	unsigned int iicds;
	unsigned int iiclc;
};

struct itop4412_i2c_xfer_data {
	struct i2c_msg *msgs;
	int msn_num;
	int cur_msg;
	int cur_ptr;
	int state;
	int err;
	wait_queue_head_t wait;
};

static struct itop4412_i2c_xfer_data itop4412_i2c_xfer_data;

static struct itop4412_i2c_regs *itop4412_i2c_regs;

static void itop4412_i2c_start(void)
{
	itop4412_i2c_xfer_data.state = STATE_START;
	
	if (itop4412_i2c_xfer_data.msgs->flags & I2C_M_RD) /* 读 */
	{
		itop4412_i2c_regs->iicds		 = itop4412_i2c_xfer_data.msgs->addr << 1;
		itop4412_i2c_regs->iicstat 	 = 0xb0;	// 主机接收，启动
	}
	else /* 写 */
	{
		itop4412_i2c_regs->iicds		 = itop4412_i2c_xfer_data.msgs->addr << 1;
		itop4412_i2c_regs->iicstat    = 0xf0; 		// 主机发送，启动
	}
}

static void itop4412_i2c_stop(int err)
{
	itop4412_i2c_xfer_data.state = STATE_STOP;
	itop4412_i2c_xfer_data.err   = err;

	drv_pr("STATE_STOP, err = %d\n", err);


	if (itop4412_i2c_xfer_data.msgs->flags & I2C_M_RD) /* 读 */
	{
		// 下面两行恢复I2C操作，发出P信号
		itop4412_i2c_regs->iicstat = 0x90;
		itop4412_i2c_regs->iiccon  = 0xaf;
		ndelay(50);  // 等待一段时间以便P信号已经发出
	}
	else /* 写 */
	{
		// 下面两行用来恢复I2C操作，发出P信号
		itop4412_i2c_regs->iicstat = 0xd0;
		itop4412_i2c_regs->iiccon  = 0xaf;
		ndelay(50);  // 等待一段时间以便P信号已经发出
	}

	/* 唤醒 */
	wake_up(&itop4412_i2c_xfer_data.wait);
	
}

static int itop4412_i2c_xfer(struct i2c_adapter *adap,
			struct i2c_msg *msgs, int num)
{
	unsigned long timeout;
	
	/* 把num个msg的I2C数据发送出去/读进来 */
	itop4412_i2c_xfer_data.msgs    = msgs;
	itop4412_i2c_xfer_data.msn_num = num;
	itop4412_i2c_xfer_data.cur_msg = 0;
	itop4412_i2c_xfer_data.cur_ptr = 0;
	itop4412_i2c_xfer_data.err     = -ENODEV;

	itop4412_i2c_start();

	/* 休眠 */
	timeout = wait_event_timeout(itop4412_i2c_xfer_data.wait, (itop4412_i2c_xfer_data.state == STATE_STOP), HZ * 5);
	if (0 == timeout)
	{
		drv_pr("itop4412_i2c_xfer time out\n");
		return -ETIMEDOUT;
	}
	else
	{
		return itop4412_i2c_xfer_data.err;
	}
}

static u32 itop4412_i2c_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL | I2C_FUNC_PROTOCOL_MANGLING;
}


static const struct i2c_algorithm itop4412_i2c_algo = {
//	.smbus_xfer     = ,
	.master_xfer	= itop4412_i2c_xfer,
	.functionality	= itop4412_i2c_func,
};

/* 1. 分配/设置i2c_adapter
 */
static struct i2c_adapter itop4412_i2c_adapter = {
 .name			 = "itop4412_100ask",
 .algo			 = &itop4412_i2c_algo,
 .owner 		 = THIS_MODULE,
};

static int isLastMsg(void)
{
	return (itop4412_i2c_xfer_data.cur_msg == itop4412_i2c_xfer_data.msn_num - 1);
}

static int isEndData(void)
{
	return (itop4412_i2c_xfer_data.cur_ptr >= itop4412_i2c_xfer_data.msgs->len);
}

static int isLastData(void)
{
	return (itop4412_i2c_xfer_data.cur_ptr == itop4412_i2c_xfer_data.msgs->len - 1);
}

static irqreturn_t itop4412_i2c_xfer_irq(int irq, void *dev_id)
{
	unsigned int iicSt;
    iicSt  = itop4412_i2c_regs->iicstat; 

	if(iicSt & 0x8){ drv_pr("Bus arbitration failed\n\r"); }

	switch (itop4412_i2c_xfer_data.state)
	{
		case STATE_START : /* 发出S和设备地址后,产生中断 */
		{
			drv_pr("Start\n");
			/* 如果没有ACK, 返回错误 */
			if (iicSt & S3C2410_IICSTAT_LASTBIT)
			{
				itop4412_i2c_stop(-ENODEV);
				break;
			}

			if (isLastMsg() && isEndData())
			{
				itop4412_i2c_stop(0);
				break;
			}

			/* 进入下一个状态 */
			if (itop4412_i2c_xfer_data.msgs->flags & I2C_M_RD) /* 读 */
			{
				itop4412_i2c_xfer_data.state = STATE_READ;
				goto next_read;
			}
			else
			{
				itop4412_i2c_xfer_data.state = STATE_WRITE;
			}	
		}

		case STATE_WRITE:
		{
			drv_pr("STATE_WRITE\n");
			/* 如果没有ACK, 返回错误 */
			if (iicSt & S3C2410_IICSTAT_LASTBIT)
			{
				itop4412_i2c_stop(-ENODEV);
				break;
			}

			if (!isEndData())  /* 如果当前msg还有数据要发送 */
			{
				itop4412_i2c_regs->iicds = itop4412_i2c_xfer_data.msgs->buf[itop4412_i2c_xfer_data.cur_ptr];
				itop4412_i2c_xfer_data.cur_ptr++;
				
				// 将数据写入IICDS后，需要一段时间才能出现在SDA线上
				ndelay(50);	
				
				itop4412_i2c_regs->iiccon = 0xaf;		// 恢复I2C传输
				break;				
			}
			else if (!isLastMsg())
			{
				/* 开始处理下一个消息 */
				itop4412_i2c_xfer_data.msgs++;
				itop4412_i2c_xfer_data.cur_msg++;
				itop4412_i2c_xfer_data.cur_ptr = 0;
				itop4412_i2c_xfer_data.state = STATE_START;
				/* 发出START信号和发出设备地址 */
				itop4412_i2c_start();
				break;
			}
			else
			{
				/* 是最后一个消息的最后一个数据 */
				itop4412_i2c_stop(0);
				break;				
			}

			break;
		}

		case STATE_READ:
		{
			drv_pr("STATE_READ\n");
			/* 读出数据 */
			itop4412_i2c_xfer_data.msgs->buf[itop4412_i2c_xfer_data.cur_ptr] = itop4412_i2c_regs->iicds;			
			itop4412_i2c_xfer_data.cur_ptr++;
next_read:
			if (!isEndData()) /* 如果数据没读写, 继续发起读操作 */
			{
				if (isLastData())  /* 如果即将读的数据是最后一个, 不发ack */
				{
					itop4412_i2c_regs->iiccon = 0x2f;   // 恢复I2C传输，接收到下一数据时无ACK
				}
				else
				{
					itop4412_i2c_regs->iiccon = 0xaf;   // 恢复I2C传输，接收到下一数据时发出ACK
				}				
				break;
			}
			else if (!isLastMsg())
			{
				/* 开始处理下一个消息 */
				itop4412_i2c_xfer_data.msgs++;
				itop4412_i2c_xfer_data.cur_msg++;
				itop4412_i2c_xfer_data.cur_ptr = 0;
				itop4412_i2c_xfer_data.state = STATE_START;
				/* 发出START信号和发出设备地址 */
				itop4412_i2c_start();
				break;
			}
			else
			{
				/* 是最后一个消息的最后一个数据 */
				itop4412_i2c_stop(0);
				break;								
			}
			break;
		}

		default: break;
	}

	/* 清中断 */
	itop4412_i2c_regs->iiccon &= ~(S3C2410_IICCON_IRQPEND);

	return IRQ_HANDLED;	
}


/*
 * I2C初始化
 */
static void itop4412_i2c_init(void)
{
	struct clk *clk;

	clk = clk_get(NULL, "i2c");
	clk_enable(clk);
	
    // 选择引脚功能：GPE15:IICSDA, GPE14:IICSCL
//    s3c_gpio_cfgpin(S3C2410_GPE(14), S3C2410_GPE14_IICSCL);
//	s3c_gpio_cfgpin(S3C2410_GPE(15), S3C2410_GPE15_IICSDA);

    /* bit[7] = 1, 使能ACK
     * bit[6] = 0, IICCLK = PCLK/16
     * bit[5] = 1, 使能中断
     * bit[3:0] = 0xf, Tx clock = IICCLK/16
     * PCLK = 50MHz, IICCLK = 3.125MHz, Tx Clock = 0.195MHz
     */
    itop4412_i2c_regs->iiccon = (1<<7) | (0<<6) | (1<<5) | (0xf);  // 0xaf

    itop4412_i2c_regs->iicadd  = 0x10;     // itop4412 slave address = [7:1]
    itop4412_i2c_regs->iicstat = 0x10;     // I2C串行输出使能(Rx/Tx)
}

static int __init i2c_bus_itop4412_init(void)
{
	/* 2. 硬件相关的设置 */
	itop4412_i2c_regs = ioremap(0x54000000, sizeof(struct itop4412_i2c_regs));
	
	itop4412_i2c_init();

	request_irq(IRQ_IIC, itop4412_i2c_xfer_irq, 0, "itop4412-i2c", NULL);

	init_waitqueue_head(&itop4412_i2c_xfer_data.wait);
	
	/* 3. 注册i2c_adapter */
	i2c_add_adapter(&itop4412_i2c_adapter);
	
	return 0;
}

static void i2c_bus_itop4412_exit(void)
{
	i2c_del_adapter(&itop4412_i2c_adapter);	
	free_irq(IRQ_IIC, NULL);
	iounmap(itop4412_i2c_regs);
}

module_init(i2c_bus_itop4412_init);
module_exit(i2c_bus_itop4412_exit);

