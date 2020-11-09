/***************************************************/ 
/*
SPIMISO0      GPE11
SPIMOSI0      GPE12
SPICLK0          GPE13
nSS0               GPG2

*/

#include <linux/irq.h>  
#include <linux/delay.h>  
#include <asm/irq.h>  
#include <linux/interrupt.h>  
#include <mach/regs-gpio.h>  
#include <mach/hardware.h>  
#include <linux/kernel.h>  
#include <linux/module.h>  
#include <linux/init.h>  
#include <linux/mm.h>  
#include <linux/fs.h>  
#include <linux/types.h>  
#include <linux/delay.h>  
#include <linux/moduleparam.h>  
#include <linux/slab.h>  
#include <linux/errno.h>  
#include <linux/ioctl.h>  
#include <linux/cdev.h>  
#include <linux/string.h>  
#include <linux/list.h>  
#include <linux/pci.h>  
#include <linux/gpio.h>  
#include <asm/uaccess.h>  
#include <asm/atomic.h>  
#include <asm/unistd.h>  
#include <linux/spinlock.h>  
#include <asm/system.h>  
#include <asm/uaccess.h>  
#include <linux/device.h>
#include <linux/miscdevice.h>   

    
#define DEVICE_NAME     "s3c24xx_spi"

struct global_mem
{
	char dataTx[128];
	char dataRx[128];
};

struct global_mem *global_memp;

/*SPI Status Register:  Data Tx/Rx ready*/
#define SPI_TXRX_READY ((readl(SPSTA0)&0x1)==0x1)//发送接收标志位判断

//GPG  GPE SPI Register address declaration
//GPG
#define GPGCON (unsigned long)ioremap(0x56000060,4)
#define GPGDAT (unsigned long)ioremap(0x56000064,4)
#define GPGUP  (unsigned long)ioremap(0x56000068,4)

//GPE
#define GPECON (unsigned long)ioremap(0x56000040,4)
#define GPEDAT (unsigned long)ioremap(0x56000044,4)
#define GPEUP  (unsigned long)ioremap(0x56000048,4)

//SPI
#define SPCON0  (unsigned long)ioremap(0x59000000,4)
#define SPSTA0  (unsigned long)ioremap(0x59000004,4)
#define SPPIN0  (unsigned long)ioremap(0x59000008,4)
#define SPPRE0  (unsigned long)ioremap(0x5900000c,4)
#define SPTDAT0 (unsigned long)ioremap(0x59000010,1)
#define SPRDAT0 (unsigned long)ioremap(0x59000014,1)

//CLK control
#define CLKCON (unsigned long)ioremap(0x4c00000c,4)
#define SCRPND  (unsigned long)ioremap(0x4a000000,4)
#define INTPND  (unsigned long)ioremap(0x4a000010,4)
#define INTMSK  (unsigned long)ioremap(0x4a000008,4)

/********************************************************/ 

static void s3c24xx_spi_config(void)
{
	unsigned int port_status;
	/***********************************************
	*PCLK  
	 ************************************************/ 
	/*Control PCLK into SPI block*/ 	
	if(!(CLKCON&(1<<18)))
 	{
  		port_status=readl(CLKCON);
  		port_status|=(1<<18);
  		writel(port_status,CLKCON);//时钟使能
 	}
 	
 	/***********************************************/

	/*config SCK0,MOSI0,MISO0 nSS0(GPG2)*/ 
	port_status=readl(GPGCON);
	port_status&=~S3C2410_GPG2_nSS0;
	port_status|=S3C2410_GPG2_nSS0;
	writel(port_status,GPGCON);
	
	/*config SPISCLK0, SPIMOSI0,SPIMISO0 )*/ 
	port_status=readl(GPECON);
	port_status&=~(S3C2410_GPE11_SPIMISO0|S3C2410_GPE12_SPIMOSI0|S3C2410_GPE13_SPICLK0);
	port_status|=S3C2410_GPE11_SPIMISO0|S3C2410_GPE12_SPIMOSI0|S3C2410_GPE13_SPICLK0;
	writel(port_status,GPECON);
	
	/*Disable Pull up*/
	port_status=0x0;
	port_status=readl(GPEUP);
	port_status&=~(1<<13|1<<12|1<<11);
	writel(port_status,GPEUP);//GPE_UP
	
	/*Disable Pull up*/
	port_status=0x0;
	port_status=readl(GPGUP);
	port_status&=~(1<<2);
	writel(port_status,GPGUP);//GPG_UP
	
	/*SPI Baud Rate Prescaler Register*/
	port_status=readl(SPPRE0);//SPI Baud speed
	port_status=0x31;//PCLK=50MHz SPICLK=PCLK/2/(value+1)=1MHz, 一般采用250K   必须小于25MHz
	 writel(port_status,SPPRE0);
	
	/* SPI Control register*/
	port_status=readl(SPCON0);
	port_status= (0<<0|0<<1|1<<2|1<<3|1<<4|0<<5|0<<6);
	writel(port_status,SPCON0);
	
	/*SPI PIN Control Register*/ 
	port_status=readl(SPPIN0);
	port_status&=~(0<<0|1<<1|1<<2);
	port_status|=(1<<2|1<<1|0<<0);
	writel(port_status,SPPIN0);//SPI pin setting	

	//printk("s3c24xx_spi_config  spi successfully\n");
}

static int s3c24xx_spi_open(struct inode *inode,struct file *filp)  
{  
	 filp-> private_data = global_memp;
	s3c24xx_spi_config();  
	printk("s3c24xx_spi_open   \n");
	return 0;  
}  
  

static int s3c24xx_spi_release(struct inode *inode,struct file *filp)  
{  
	printk("s3c24xx_spi_release   \n");
	return 0;  
}  
   

static void writeByte(const char c)  
{  
	int j = 0;  	
	//for(j=0; j<0xFF; j++);  

	while(!SPI_TXRX_READY)  
		for(j=0; j<0xFF; j++);  		
	writeb(c, SPTDAT0);
}  

   

static char readByte(void)  
{  

	int j = 0;  
	    char ch = 0; 

	while(!SPI_TXRX_READY)  
		for(j=0;j<0xFF;j++);  

	ch=readl(SPRDAT0);  
	return ch;  
}  

static ssize_t s3c24xx_spi_read(struct file *filp,char __user *buf,size_t count,loff_t *f_ops)  
{  
	ssize_t ret;
	int i;
	printk("<1>spi read\n");  
    
	struct global_mem *mem = filp-> private_data;
    
	for(i=0; i<count; i++)  
	{  
		mem->dataRx[i] = readByte();    
		printk("read 0x%02X\n", mem->dataRx[i]);  
	}  
		
	if (copy_to_user(buf, mem->dataRx, count))
		ret =  - EFAULT;
	else
		ret = count;
	return ret;  
}  

   

static ssize_t s3c24xx_spi_write(struct file *filp,const char __user *buf,size_t count,loff_t *f_ops)  
{  
	int i;  
   
	struct global_mem *mem = filp-> private_data;

	printk("<1>spi write!,count=%d\n",count);  

	if(copy_from_user(mem->dataTx, buf, count))  
	{  
		printk("no enough memory!\n");  
		return -1;  
	}         

	for(i=0;i<count;i++)  
	{  
		writeByte(mem->dataTx[i]);  
		printk("write 0x%02X\n", mem->dataTx[i]);  
	}  
	return count;  
}  

   

static ssize_t s3c24xx_spi_ioctl(struct inode *inode,struct file *filp,unsigned int cmd,unsigned long data)  
{  
	return 0;  
}  

static const struct file_operations spi_fops = {  
	.owner=THIS_MODULE,  
	.open=s3c24xx_spi_open,  
	.read=s3c24xx_spi_read,  
	.write=s3c24xx_spi_write,  
	.ioctl=s3c24xx_spi_ioctl,  
	.release=s3c24xx_spi_release, 
};
   
static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &spi_fops,
};
   

static int __init spi_init(void)  
{	  
	int ret;
	
	global_memp=kmalloc(sizeof(struct global_mem),GFP_KERNEL);
	if(!global_memp)//申请失败
	{
		ret=-ENOMEM;
		goto fail_malloc;
	}
	memset(global_memp,0,sizeof(struct global_mem)); 

	ret = misc_register(&misc);
	printk (DEVICE_NAME"\tinitialized\n");
fail_malloc:	
    	return ret;
}  

static void __exit spi_exit(void)
{
	misc_deregister(&misc);
	kfree(global_memp);
	global_memp = NULL;
}   

module_init(spi_init);  
module_exit(spi_exit);     

MODULE_LICENSE("GPL");  
MODULE_AUTHOR("Hanson He by www.embedclub.com");  
MODULE_DESCRIPTION("SPI driver for S3C24xx"); 
