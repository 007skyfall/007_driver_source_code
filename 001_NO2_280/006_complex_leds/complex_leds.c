/*	实现功能：1./dev/leds节点下，两个LED均控制亮灭。
*             2./dev/led2下的节点，LED2被控制亮灭。     
*             3./dev/led3下的节点，LED3被控制亮灭。  
*/ 

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/device.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif

#define LED_NAME     "leds"  /* 加载模式后，执行”cat /proc/devices”命令看到的设备名称 */
#define LED_MAJOR       231     /* 主设备号 */
#define GPK1_CON		(0x11000060)
#define GPL2_CON		(0x11000100)
#define GPIOSIZE				8


static struct class     *leds_class 	= NULL;
static struct device	*leds_device[3]	= {NULL, NULL, NULL};

volatile unsigned long *gpl2con = NULL;
volatile unsigned long *gpl2dat = NULL;

volatile unsigned long *gpk1con = NULL;
volatile unsigned long *gpk1dat = NULL;

static char leds_status = 0x0;  
static DEFINE_SEMAPHORE(leds_lock); // 定义赋值

//static int minor;

/* 应用程序对设备文件/dev/leds执行open(...)时，
 * 就会调用 itop4412_leds_open函数
 */
static int itop4412_leds_open(struct inode *inode, struct file *file)
{
	int minor = MINOR(inode->i_rdev); //MINOR(inode->i_cdev);

	switch(minor)
	{
        case 1: /* /dev/leds */
        {
         	/* 配置GPL2_0为输出 */
	        *gpl2con &= ~((0xf<<(4*0)))|(0x01<<(4*0));

	        /* 配置GPK1_1为输出 */
	        *gpk1con |= ~((0xf<<(4*1)))|(0x01<<(4*1));
            down(&leds_lock);
            leds_status = 0x0;
            up(&leds_lock);
            break;
        }

        case 2: /* /dev/led2 */
        {
         	/* 配置GPL2_0为输出 */
	        *gpl2con &= ~((0xf<<(4*0)))|(0x01<<(4*0));
            down(&leds_lock);
            leds_status &= ~(1<<0);
            up(&leds_lock);
            break;
        }

        case 3: /* /dev/led3 */
        {

	        /* 配置GPK1_1为输出 */
	        *gpk1con |= ~((0xf<<(4*1)))|(0x01<<(4*1));
            down(&leds_lock);
            leds_status &= ~(1<<1);
            up(&leds_lock);
			break;
        }

        default:
        {
            break;
        }
        
	}
	
    return 0;
}

static int itop4412_leds_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	int minor = MINOR(filp->f_dentry->d_inode->i_rdev);
    char val;

    switch (minor)
    {
        case 1: /* /dev/leds */
        {
            
            copy_to_user(buff, (const void *)&leds_status, 1);                    
            break;
        }

        case 2: /* /dev/led2 */
        {
            down(&leds_lock);
            val = leds_status & 0x1;
            up(&leds_lock);
            copy_to_user(buff, (const void *)&val, 1);
            break;
        }

        case 3: /* /dev/led3 */
        {
            down(&leds_lock);
            val = (leds_status>>1) & 0x1;
            up(&leds_lock);
            copy_to_user(buff, (const void *)&val, 1);
            break;
        }

        default:
        {
            break;
        }

    }

    return 1;
}

static ssize_t itop4412_leds_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
    //int minor = MINOR(inode->i_rdev); //MINOR(inode->i_cdev);
	int minor = MINOR(file->f_dentry->d_inode->i_rdev);
    char val;

    copy_from_user(&val, buf, 1);

//	drv_pr("------val=%d------\n",val);
//	drv_pr("------minor=%d------\n",minor);
	switch(val)
		{
			case 0:
				
			switch (minor)
		    {
		        case 1: /* /dev/leds */
		        {            
		   		    // LED2点灯
			    	*gpl2dat |= (0x01<<0);

				    // LED3点灯
				    *gpk1dat |= (0x01<<1);

		            down(&leds_lock);
		            leds_status = val;
		            up(&leds_lock);
		            break;
		        }

		        case 2: /* /dev/led2 */
		        {
		            // LED2点灯
			    	*gpl2dat |= (0x01<<0);

				    // LED3灭灯
				    *gpk1dat &= ~(0x01<<1);

		            if (val == 0)
		            {
		                down(&leds_lock);
		                leds_status &= ~(1<<0);
		                up(&leds_lock);
		            }
		            else
		            {
		                down(&leds_lock);
		                leds_status |= (1<<0);                
		                up(&leds_lock);
		            }
		            break;
		        }

		        case 3: /* /dev/led3 */
		        {

		            // LED2灭灯
			    	*gpl2dat &= ~(0x01<<0);

				    // LED3点灯
				    *gpk1dat |= (0x01<<1);

		            if (val == 0)
		            {
		                down(&leds_lock);
		                leds_status &= ~(1<<1);
		                up(&leds_lock);
		            }
		            else
		            {
		                down(&leds_lock);
		                leds_status |= (1<<1);                
		                up(&leds_lock);
		            }
		            break;
		        }
		        
		    }
			break;

			case 1:
				
				switch (minor)
		        case 1: /* /dev/leds */
		        {            
		   		    // LED2灭灯
			    	*gpl2dat &= ~(0x01<<0);

				    // LED3灭灯
				    *gpk1dat &= ~(0x01<<1);

		            down(&leds_lock);
		            leds_status = val;
		            up(&leds_lock);
		            break;
		        }

		        case 2: /* /dev/led2 */
		        {
		            // LED2灭灯
			    	*gpl2dat &= ~(0x01<<0);

				    // LED3灭灯
				    *gpk1dat &= ~(0x01<<1);

		            if (val == 0)
		            {
		                down(&leds_lock);
		                leds_status &= ~(1<<0);
		                up(&leds_lock);
		            }
		            else
		            {
		                down(&leds_lock);
		                leds_status |= (1<<0);                
		                up(&leds_lock);
		            }
		            break;
		        }

		        case 3: /* /dev/led3 */
		        {

		            // LED2灭灯
			    	*gpl2dat &= ~(0x01<<0);

				    // LED3灭灯
				    *gpk1dat &= ~(0x01<<1);

		            if (val == 0)
		            {
		                down(&leds_lock);
		                leds_status &= ~(1<<1);
		                up(&leds_lock);
		            }
		            else
		            {
		                down(&leds_lock);
		                leds_status |= (1<<1);                
		                up(&leds_lock);
		            }
		            break;
		        }
		        break;

				default:
					break;
		    }


    return 1;
}


/* 这个结构是字符设备驱动程序的核心
 * 当应用程序操作设备文件时所调用的open、read、write等函数，
 * 最终会调用这个结构中指定的对应函数
 */
static struct file_operations itop4412_leds_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   itop4412_leds_open,     
	.read	=	itop4412_leds_read,	   
	.write	=	itop4412_leds_write,	   
};

/*
 * 执行insmod命令时就会调用这个函数 
 */
static int __init itop4412_leds_init(void)
{
    int ret;
	int minor = 0;

	gpl2con = (volatile unsigned long *)ioremap(GPL2_CON, GPIOSIZE);
	gpl2dat = gpl2con + 1;

	gpk1con = (volatile unsigned long *)ioremap(GPK1_CON, GPIOSIZE);
	gpk1dat = gpk1con + 1;
	
	drv_pr("%s : %s : %d\n",__FILE__, __func__, __LINE__);
    /* 注册字符设备
     * 参数为主设备号、设备名字、file_operations结构；
     * 这样，主设备号就和具体的file_operations结构联系起来了，
     * 操作主设备为LED_MAJOR的设备文件时，就会调用itop4412_leds_fops中的相关成员函数
     * LED_MAJOR可以设为0，表示由内核自动分配主设备号
     */
    ret = register_chrdev(LED_MAJOR, LED_NAME, &itop4412_leds_fops);
    if (ret < 0) {
      drv_pr(" can't register major number!\n");
      return ret;
    }

	leds_class = class_create(THIS_MODULE, LED_NAME);
	if (IS_ERR(leds_class))
		return PTR_ERR(leds_class);
    
	leds_device[0] = device_create(leds_class, NULL, MKDEV(LED_MAJOR, 1), NULL, "leds"); /* /dev/leds */
	
	for (minor=1; minor<3; minor++)  /* /dev/led<2|3> */
	{
		leds_device[minor] = device_create(leds_class, NULL, MKDEV(LED_MAJOR, minor+1), NULL, "led%d", minor+1);
		if (unlikely(IS_ERR(leds_device[minor])))
			return PTR_ERR(leds_device[minor]);
	}
        
    drv_pr("initialized!\n");

    return 0;
}

/*
 * 执行rmmod命令时就会调用这个函数 
 */
static void __exit itop4412_leds_exit(void)
{
	int minor;
    /* 卸载驱动程序 */
    unregister_chrdev(LED_MAJOR, LED_NAME);
	for (minor=1; minor<4; minor++){
        device_destroy(leds_class, MKDEV(LED_MAJOR, minor));
	}

	class_destroy(leds_class);

    iounmap(gpl2con);
    iounmap(gpk1con);
}

/* 这两行指定驱动程序的初始化函数和卸载函数 */
module_init(itop4412_leds_init);
module_exit(itop4412_leds_exit);

/* 描述驱动程序的一些信息，不是必须的 */
MODULE_AUTHOR("seedling");
MODULE_VERSION("v1.0");
MODULE_DESCRIPTION("ITOP4412 LED Driver");

