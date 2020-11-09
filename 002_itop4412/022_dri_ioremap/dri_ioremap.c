#include <linux/init.h>
#include <linux/module.h>
#include <asm/io.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("SKYFALL");

//用于存放虚拟地址和物理地址
volatile unsigned long virt_addr_led2,phys_addr_led2,virt_addr_led3,phys_addr_led3;
//用于存放LED2的三个寄存器的地址
volatile unsigned long *GPL2CON,*GPL2DAT,*GPL2PUD;
//用于存放LED3的三个寄存器的地址
volatile unsigned long *GPK1CON,*GPK1DAT,*GPK1PUD;

void gpl2_device_init(void)
{
	//物理地址的起始地址0x11000100→0x11000108
	phys_addr_led2 = 0x11000100;
	//0x11000100是GPL2CON的物理地址
	virt_addr_led2 =(unsigned long)ioremap(phys_addr_led2,0x10);
	//指定需要操作的寄存器地址
	GPL2CON = (unsigned long *)(virt_addr_led2+0x00);
	GPL2DAT = (unsigned long *)(virt_addr_led2+0x04);
	GPL2PUD	=(unsigned long *)(virt_addr_led2+0x08);
}

void gpk1_device_init(void)
{
	//物理地址的起始地址0x11000060→0x11000068
	phys_addr_led3 = 0x11000060;
	//0x11000100是GPL2CON的物理地址
	virt_addr_led3 =(unsigned long)ioremap(phys_addr_led3,0x10);
	//指定需要操作的寄存器地址
	GPK1CON = (unsigned long *)(virt_addr_led3+0x00);
	GPK1DAT = (unsigned long *)(virt_addr_led3+0x04);
	GPK1PUD	=(unsigned long *)(virt_addr_led3+0x08);
}


//配置开发板的GPIO寄存器---->LED2
void gpl2_configure(void)
{
	//配置为输出模式。bit3 bit2 bit1设置为0，bit0设置为1
	*GPL2CON  &= 0xfffffff1;
	*GPL2CON  |= 0x00000001;
	//GPL2PUD寄存器，bit[0:1]设为0x03,上拉模式
	*GPL2PUD |=0x0003;
}


//配置开发板的GPIO寄存器---->LED3
void gpk1_configure(void)
{
	//配置为输出模式。bit7 bit6 bit5设置为0，bit3设置为1
	*GPK1CON  &= 0xffffff1f;
	*GPK1CON  |= 0x00000010;
	//GPK1PUD寄存器，bit[7:4]设为0x03,上拉模式
	*GPK1PUD |=0x0030;
}

void gpl2_on(void)//点亮led2灯
{
	*GPL2DAT |= 0x01;
}

void gpk1_on(void)//点亮led3灯
{
	*GPK1DAT |= 0x02;
}

void gpl2_off(void)//灭掉led2
{
	*GPL2DAT &= 0xfe;
}

void gpk1_off(void)//灭掉led3
{
	*GPK1DAT &= 0xfd;
}

static int __init led_init(void)
{
	printk("led_init!\n");

	gpl2_device_init();	//实现IO内存的映射,LED2
	gpk1_device_init();	//实现IO内存的映射,LED3
	gpl2_configure();	//配置GPL2为输出模式,LED2
	gpk1_configure();	//配置GPL2为输出模式,LED3
	gpl2_on();	//LED2
	gpk1_on();	//LED3
	printk("led open!\n");

	return 0;
}


static void __exit led_exit(void)
{
	printk("led_exit!\n");

	gpl2_off(); //ELD2
	gpk1_off(); //LED3
	
	printk("led close\n");
	return ;
}

module_init(led_init);
module_exit(led_exit);
