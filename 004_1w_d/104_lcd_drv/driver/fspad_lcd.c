#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fb.h>
#include <asm/io.h> 
#include <linux/dma-mapping.h>


#define mWIDTH 1024
#define mHEIGHT 600

//设置GPIO为LCD模式
static unsigned int *GPF0CON; 
static unsigned int *GPF1CON; 
static unsigned int *GPF2CON; 
static unsigned int *GPF3CON; 

//LCD时钟配置
static unsigned int *CLK_SRC_LCD0;
static unsigned int *CLK_DIV_LCD;
static unsigned int *LCDBLK_CFG;

//LCD背光灯
static unsigned int *GPD0CON;
static unsigned int *GPD0DAT;

//lcd控制寄存器
static unsigned int *VIDCON0;
static unsigned int *VIDCON1;
static unsigned int *VIDCON2;
static unsigned int *VIDTCON0;
static unsigned int *VIDTCON1;
static unsigned int *VIDTCON2;
static unsigned int *WINCON0;
static unsigned int *SHADOWCON;
static unsigned int *WINCHMAP2;
static unsigned int *VIDOSD0A;
static unsigned int *VIDOSD0B;
static unsigned int *VIDOSD0C;
static unsigned int *VIDW0nADD0B0;
static unsigned int *VIDW0nADD1B0;
static unsigned int *VIDW0nADD2;

static struct fb_info *fspad_lcd_info = NULL;
static u32	pseudo_palette[16] = {0};
dma_addr_t map_dma;
unsigned int map_size;

static inline unsigned int chan_to_field(unsigned int chan,
					 struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}

static int fspad_fb_setcolreg(unsigned regno,
			       unsigned red, unsigned green, unsigned blue,
			       unsigned transp, struct fb_info *info)
{
	unsigned int val = 0;
	
	if (regno < 16) {
		u32 *pal = fspad_lcd_info->pseudo_palette;

		val  = chan_to_field(red,   &fspad_lcd_info->var.red);
		val |= chan_to_field(green, &fspad_lcd_info->var.green);
		val |= chan_to_field(blue,  &fspad_lcd_info->var.blue);
		pal[regno] = val;
	}
	return 0;
}

struct fb_ops fspad_fb_ops = {
	.owner		= THIS_MODULE,
	.fb_setcolreg	= fspad_fb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

static void lcd_controller_init(void)
{
	unsigned int map = PAGE_ALIGN(fspad_lcd_info->fix.smem_len);
	map_size = map;

	/*
		VIDCON0
		bit[31]:保留位必须设置为0
		bit[30]:使能MIPI DSI(这里就不使能了，设置为0)
		bit[29]:保留位设置为0
		bit[28:26]:决定视频控制器的输出格式(选择默认RGB接口:000)
		bit[25:23]: indirect i80 interface(不需要，设置为000)
		bit[22:20]: indirect i80 interface(不需要，设置为000)
		bit[19]:保留设置为0
		bit[18]:选择显示模式(默认选择并行0)
		bit[17]:Controls inverting RGB_ORDER(选择0)
		bit[16]:Selects CLKVAL_F update timing control. (选择0)
		bit[15:14]:保留位
		bit[13:6]:决定VCLK和CLKVAL[7:0]
				  VCLK = FIMD * SCLK/(CLKVAL+1)
				  从芯片手册上我们查询到VCLK典型值为51.2MHz
				  51.2MHz = 800MHz/(CLKVAL+1),CLKVAL = 14
		bit[5]:vlck时钟自由模式，不开启0
		bit[4]:保留设置为0
		bit[1]:ENVID (1)
		bit[0]:ENVID_F (1)
		Display On: ENVID and ENVID_F are set to "1"
	*/
	*VIDCON0 = 0;
	*VIDCON0 |= (14<<6)|(0<<1)|(0<<0);
	/*
		VIDCON1
		bit[26:16]:只读寄存器，里面存放的是当前所在行的位置，从0开始向上计数最大到1024
		bit[15]:状态只读寄存器
		bit[14:13]:只读寄存器
		bit[12:11]:保留
		bit[10:9]:FIXVCLK 01 = VCLK running (选择01让Vclk运行)
		bit[8]:保留
		bit[7]:vclk在下降沿取数据还是在上升沿取数据(从LCD芯片手册知道下降沿取数据)0
		bit[6]:行电平是否翻转 (1翻转)
		bit[5]:场电平是否翻转 (1翻转)
		bit[4]:VDEN信号是否翻转(0不需要翻转)
		bit[3:0]:保留
	*/
	*VIDCON1 &= ~(0xfff);
	*VIDCON1 |= (1<<9)|(0<<7)|(1<<6)|(1<<5)|(0<<4);
	/*
		VIDCON2
		这个寄存器我们选择默认就行了，但是注意有一个保留位必须设置为1
		bit[15:14]:Reserved This bit should be set to 1. 
	*/
	*VIDCON2 = 0;
	*VIDCON2 |= (3<<14);
	/*
		VIDCON3不需要设置
	*/
	/*
		VIDTCON0
		bit[31:24]:VBPDE yuyv接口暂时不设置
		bit[23:16]:VBPD+1 = 23;VBPD = 22;
		bit[15:8]:VFPD+1 = 12;VFPD=11;
		bit[7:0]:VSPW :场同步信号触发电平时间VSPW+1 = 10ns(1-20),VSPW = 9;
	*/
	*VIDTCON0 =0;
	*VIDTCON0 |= (22<<16)|(11<<8)|(9<<0);
	
	/*
		VIDTCON1
		bit[31:24]:VFPDE yuyv接口暂时不设置
		bit[23:16]:HBPD+1 = 160;VBPD = 159;
		bit[15:8]:HFPD+1 = 160;VFPD=159;
		bit[7:0]:HSPW :场同步信号触发电平时间HSPW+1 = 70ns(1-140),VSPW = 69;
	*/
	*VIDTCON1 = 0;
	*VIDTCON1 |= (159<<16)|(159<<8)|(15<<0);

	/*
		VIDTCON2
		bit[21:11]:LINEVAL多少行:LINEVAL+1 = 600,LINEVAL=599
		bit[10:0]:HOZVAL多少列HOZVAL+1 = 1024,HOZVAL =1023
	*/
	*VIDTCON2 = 0;
	*VIDTCON2 |= (599<<11)|(1023<<0);
	/*
		VIDTCON3
		不知道如何使用，暂不设置
	*/

	/*
		WINCON0
		bit[15]:开启字节转换空间1
		bit[5:2]:设置像素格式为RGB565(0x0101)
		bit[0]:开启图像输出使能1
	*/
	*WINCON0 = 0; 
	*WINCON0 |= (1<<16)|(0<<9)|(5<<2);

	/*
		bit[18:16]:选择通道0给窗口0使用
		bit[2:0]:选择窗口0的0通道
		注意:这个寄存器选择默认就行了
	*WINCHMAP2 |=(0x1<<16)|(0x1<<0); 
	*/
	
	/*
		VIDOSD0A
		指定OSD图像左上像素的水平屏幕坐标
		bit[21:11]:左上x(这里设置为0)
		bit[10:0]:左上Y(这里设置为0)
	*/
	*VIDOSD0A = 0;
	/*
		VIDOSD0B
		指定OSD图像右下像素的水平屏幕坐标
		bit[21:11]:右下x (1023)
		bit[10:0]:右下y (599)
	*/
	*VIDOSD0B = 0;
	*VIDOSD0B |= (1023<<11)|(599<<0);
	/*
		bit[25:24]:保留
		bit[23:0]:设置图片大小
		1024*600 
	*/
	*VIDOSD0C = 0;
	*VIDOSD0C |= (1024*600);
#if 1
	fspad_lcd_info->screen_base = dma_alloc_writecombine(NULL,map_size,&map_dma,GFP_KERNEL);
	if(!(fspad_lcd_info->screen_base)){
		printk("alloc screen_base is fail.\n");
//		return -ENOMEM;
	}
	memset(fspad_lcd_info->screen_base, 0x0, map_size);
	fspad_lcd_info->fix.smem_start = map_dma; 
#endif


	/*
		bit[31:0]:物理的启始地址
	*/
	*VIDW0nADD0B0 =fspad_lcd_info->fix.smem_start;

	/*
		bit[31:0]:虚拟地址的结束地址
	*/
	*VIDW0nADD1B0 = fspad_lcd_info->fix.smem_start+fspad_lcd_info->fix.smem_len;

	/*
		bit[25:13]:虚拟内存的偏移
		bit[12:0]:虚拟内存的页宽
	*/
	*VIDW0nADD2 = 0;
	*VIDW0nADD2 |= ((1024*2)<<0);

	//开启背光灯
	*GPD0DAT |= 1<<1; 

	/*
		SHADOWCON
		使能通道0
	*/
	*SHADOWCON |= 1;

	//前面的寄存器用于使能
	*VIDCON0 |= (1<<1)|(1<<0);
	*WINCON0 |= (1<<0);

	
}

static int __init fspad_lcd_init(void)
{
	int ret;

	/*1.为fb_info结构体分配空间*/
	fspad_lcd_info = framebuffer_alloc(0,NULL);
	if(fspad_lcd_info == NULL){
		printk("alloc memnory is fail.\n");
		return -ENOMEM;
	}

	/*2.填充结构体*/
	/*2.1设置固定参数*/
//	fspad_lcd_info->flags = FBINFO_FLAG_DEFAULT;
	strcpy(fspad_lcd_info->fix.id,"mylcd");

	//内存大小用字节来表示
	fspad_lcd_info->fix.smem_len = mWIDTH*mHEIGHT*2; 
	fspad_lcd_info->fix.type = FB_TYPE_PACKED_PIXELS;
	fspad_lcd_info->fix.visual = FB_VISUAL_TRUECOLOR;
	fspad_lcd_info->fix.line_length = mWIDTH*2;
	
	/*2.2设置可变参数*/
	fspad_lcd_info->var.xres = mWIDTH;
	fspad_lcd_info->var.yres = mHEIGHT;
	fspad_lcd_info->var.xres_virtual= mWIDTH;
	fspad_lcd_info->var.yres_virtual= mHEIGHT;
	fspad_lcd_info->var.xoffset= 0;
	fspad_lcd_info->var.yoffset= 0;
	fspad_lcd_info->var.bits_per_pixel = 16;
	//灰度默认为0
	fspad_lcd_info->var.grayscale = 0;
	fspad_lcd_info->var.red.offset = 11;
	fspad_lcd_info->var.red.length = 5;
	fspad_lcd_info->var.green.offset = 5;
	fspad_lcd_info->var.green.length = 6;
	fspad_lcd_info->var.blue.offset = 0;
	fspad_lcd_info->var.blue.length = 5;
	//设置立即激活
	fspad_lcd_info->var.activate = FB_ACTIVATE_NOW;

	/*2.3设置操作方法*/
	fspad_lcd_info->fbops = &fspad_fb_ops;


	/*2.4其他相关的设置*/
	fspad_lcd_info->screen_size = fspad_lcd_info->fix.smem_len;
	
	//调色板
	fspad_lcd_info->pseudo_palette = pseudo_palette ;


	/*3.硬件相关的操作(设置lcd控制器)*/
	CLK_SRC_LCD0 = ioremap(0x1003c234,4);
	CLK_DIV_LCD = ioremap(0x1003c534,4);
	LCDBLK_CFG = ioremap(0x10010210,4);
	GPF0CON = ioremap(0x11400180,4);
	GPF1CON = ioremap(0x114001a0,4);
	GPF2CON = ioremap(0x114001c0,4);
	GPF3CON = ioremap(0x114001e0,4);
	GPD0CON = ioremap(0x114000a0,4);
	GPD0DAT = ioremap(0x114000a4,4);
	VIDCON0 = ioremap(0x11c00000,4);
	VIDCON1 = ioremap(0x11c00004,4);
	VIDCON2 = ioremap(0x11c00008,4);
	VIDTCON0 = ioremap(0x11c00010,4);
	VIDTCON1 = ioremap(0x11c00014,4);
	VIDTCON2 = ioremap(0x11c00018,4);
	WINCON0 = ioremap(0x11c00020,4);
	SHADOWCON = ioremap(0x11c00034,4);
	WINCHMAP2 = ioremap(0x11c0003c,4);
	VIDOSD0A = ioremap(0x11c00040,4);
	VIDOSD0B = ioremap(0x11c00044,4);
	VIDOSD0C = ioremap(0x11c00048,4);
	VIDW0nADD0B0 = ioremap(0x11c000a0,4);
	VIDW0nADD1B0 = ioremap(0x11c000d0,4);
	VIDW0nADD2 = ioremap(0x11c00100,4);
	
	//lcd时钟初始化
	*CLK_SRC_LCD0 &= ~0xf;
	*CLK_DIV_LCD &= ~0xf;
	*CLK_SRC_LCD0 |= 0x6;
	*LCDBLK_CFG |= (1<<1);
	printk("lcd clk is set done.\n");
	
	//GPIO端口的初始化
	*GPF0CON = 0x22222222;
	*GPF1CON = 0x22222222;
	*GPF2CON = 0x22222222;
	*GPF3CON = 0x00222222;
	printk("lcd gpio is set done.\n");

	//背光灯初始化
	*GPD0CON &= ~0xf<<4;
	*GPD0CON |= (0x1<<4);
	printk("lcd black led is set done.\n");
	
	//lcd控制器管脚映射
	//lcd控制器的配置
	lcd_controller_init();
	printk("lcd controler is set done.\n");

	/*4.注册结构体*/
	ret = register_framebuffer(fspad_lcd_info);
	if (ret < 0) {
		printk("Failed to register framebuffer device: %d\n",ret);
		return -EAGAIN;
	}
	printk("framerbuffer register success.\n");
	return 0;
}
static void __exit fspad_lcd_exit(void)
{
	dma_free_writecombine(NULL,map_size,fspad_lcd_info->screen_base,map_dma);
	/*释放内存空间*/
	framebuffer_release(fspad_lcd_info);
	/*iounmap*/
	/*注销结构体*/
	unregister_framebuffer(fspad_lcd_info);
	printk("fspad_lcd_exit.\n");
}

module_init(fspad_lcd_init);
module_exit(fspad_lcd_exit);
MODULE_LICENSE("GPL");


