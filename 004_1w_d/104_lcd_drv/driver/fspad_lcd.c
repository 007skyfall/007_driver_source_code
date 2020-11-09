#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fb.h>
#include <asm/io.h> 
#include <linux/dma-mapping.h>


#define mWIDTH 1024
#define mHEIGHT 600

//����GPIOΪLCDģʽ
static unsigned int *GPF0CON; 
static unsigned int *GPF1CON; 
static unsigned int *GPF2CON; 
static unsigned int *GPF3CON; 

//LCDʱ������
static unsigned int *CLK_SRC_LCD0;
static unsigned int *CLK_DIV_LCD;
static unsigned int *LCDBLK_CFG;

//LCD�����
static unsigned int *GPD0CON;
static unsigned int *GPD0DAT;

//lcd���ƼĴ���
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
		bit[31]:����λ��������Ϊ0
		bit[30]:ʹ��MIPI DSI(����Ͳ�ʹ���ˣ�����Ϊ0)
		bit[29]:����λ����Ϊ0
		bit[28:26]:������Ƶ�������������ʽ(ѡ��Ĭ��RGB�ӿ�:000)
		bit[25:23]: indirect i80 interface(����Ҫ������Ϊ000)
		bit[22:20]: indirect i80 interface(����Ҫ������Ϊ000)
		bit[19]:��������Ϊ0
		bit[18]:ѡ����ʾģʽ(Ĭ��ѡ����0)
		bit[17]:Controls inverting RGB_ORDER(ѡ��0)
		bit[16]:Selects CLKVAL_F update timing control. (ѡ��0)
		bit[15:14]:����λ
		bit[13:6]:����VCLK��CLKVAL[7:0]
				  VCLK = FIMD * SCLK/(CLKVAL+1)
				  ��оƬ�ֲ������ǲ�ѯ��VCLK����ֵΪ51.2MHz
				  51.2MHz = 800MHz/(CLKVAL+1),CLKVAL = 14
		bit[5]:vlckʱ������ģʽ��������0
		bit[4]:��������Ϊ0
		bit[1]:ENVID (1)
		bit[0]:ENVID_F (1)
		Display On: ENVID and ENVID_F are set to "1"
	*/
	*VIDCON0 = 0;
	*VIDCON0 |= (14<<6)|(0<<1)|(0<<0);
	/*
		VIDCON1
		bit[26:16]:ֻ���Ĵ����������ŵ��ǵ�ǰ�����е�λ�ã���0��ʼ���ϼ������1024
		bit[15]:״ֻ̬���Ĵ���
		bit[14:13]:ֻ���Ĵ���
		bit[12:11]:����
		bit[10:9]:FIXVCLK 01 = VCLK running (ѡ��01��Vclk����)
		bit[8]:����
		bit[7]:vclk���½���ȡ���ݻ�����������ȡ����(��LCDоƬ�ֲ�֪���½���ȡ����)0
		bit[6]:�е�ƽ�Ƿ�ת (1��ת)
		bit[5]:����ƽ�Ƿ�ת (1��ת)
		bit[4]:VDEN�ź��Ƿ�ת(0����Ҫ��ת)
		bit[3:0]:����
	*/
	*VIDCON1 &= ~(0xfff);
	*VIDCON1 |= (1<<9)|(0<<7)|(1<<6)|(1<<5)|(0<<4);
	/*
		VIDCON2
		����Ĵ�������ѡ��Ĭ�Ͼ����ˣ�����ע����һ������λ��������Ϊ1
		bit[15:14]:Reserved This bit should be set to 1. 
	*/
	*VIDCON2 = 0;
	*VIDCON2 |= (3<<14);
	/*
		VIDCON3����Ҫ����
	*/
	/*
		VIDTCON0
		bit[31:24]:VBPDE yuyv�ӿ���ʱ������
		bit[23:16]:VBPD+1 = 23;VBPD = 22;
		bit[15:8]:VFPD+1 = 12;VFPD=11;
		bit[7:0]:VSPW :��ͬ���źŴ�����ƽʱ��VSPW+1 = 10ns(1-20),VSPW = 9;
	*/
	*VIDTCON0 =0;
	*VIDTCON0 |= (22<<16)|(11<<8)|(9<<0);
	
	/*
		VIDTCON1
		bit[31:24]:VFPDE yuyv�ӿ���ʱ������
		bit[23:16]:HBPD+1 = 160;VBPD = 159;
		bit[15:8]:HFPD+1 = 160;VFPD=159;
		bit[7:0]:HSPW :��ͬ���źŴ�����ƽʱ��HSPW+1 = 70ns(1-140),VSPW = 69;
	*/
	*VIDTCON1 = 0;
	*VIDTCON1 |= (159<<16)|(159<<8)|(15<<0);

	/*
		VIDTCON2
		bit[21:11]:LINEVAL������:LINEVAL+1 = 600,LINEVAL=599
		bit[10:0]:HOZVAL������HOZVAL+1 = 1024,HOZVAL =1023
	*/
	*VIDTCON2 = 0;
	*VIDTCON2 |= (599<<11)|(1023<<0);
	/*
		VIDTCON3
		��֪�����ʹ�ã��ݲ�����
	*/

	/*
		WINCON0
		bit[15]:�����ֽ�ת���ռ�1
		bit[5:2]:�������ظ�ʽΪRGB565(0x0101)
		bit[0]:����ͼ�����ʹ��1
	*/
	*WINCON0 = 0; 
	*WINCON0 |= (1<<16)|(0<<9)|(5<<2);

	/*
		bit[18:16]:ѡ��ͨ��0������0ʹ��
		bit[2:0]:ѡ�񴰿�0��0ͨ��
		ע��:����Ĵ���ѡ��Ĭ�Ͼ�����
	*WINCHMAP2 |=(0x1<<16)|(0x1<<0); 
	*/
	
	/*
		VIDOSD0A
		ָ��OSDͼ���������ص�ˮƽ��Ļ����
		bit[21:11]:����x(��������Ϊ0)
		bit[10:0]:����Y(��������Ϊ0)
	*/
	*VIDOSD0A = 0;
	/*
		VIDOSD0B
		ָ��OSDͼ���������ص�ˮƽ��Ļ����
		bit[21:11]:����x (1023)
		bit[10:0]:����y (599)
	*/
	*VIDOSD0B = 0;
	*VIDOSD0B |= (1023<<11)|(599<<0);
	/*
		bit[25:24]:����
		bit[23:0]:����ͼƬ��С
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
		bit[31:0]:�������ʼ��ַ
	*/
	*VIDW0nADD0B0 =fspad_lcd_info->fix.smem_start;

	/*
		bit[31:0]:�����ַ�Ľ�����ַ
	*/
	*VIDW0nADD1B0 = fspad_lcd_info->fix.smem_start+fspad_lcd_info->fix.smem_len;

	/*
		bit[25:13]:�����ڴ��ƫ��
		bit[12:0]:�����ڴ��ҳ��
	*/
	*VIDW0nADD2 = 0;
	*VIDW0nADD2 |= ((1024*2)<<0);

	//���������
	*GPD0DAT |= 1<<1; 

	/*
		SHADOWCON
		ʹ��ͨ��0
	*/
	*SHADOWCON |= 1;

	//ǰ��ļĴ�������ʹ��
	*VIDCON0 |= (1<<1)|(1<<0);
	*WINCON0 |= (1<<0);

	
}

static int __init fspad_lcd_init(void)
{
	int ret;

	/*1.Ϊfb_info�ṹ�����ռ�*/
	fspad_lcd_info = framebuffer_alloc(0,NULL);
	if(fspad_lcd_info == NULL){
		printk("alloc memnory is fail.\n");
		return -ENOMEM;
	}

	/*2.���ṹ��*/
	/*2.1���ù̶�����*/
//	fspad_lcd_info->flags = FBINFO_FLAG_DEFAULT;
	strcpy(fspad_lcd_info->fix.id,"mylcd");

	//�ڴ��С���ֽ�����ʾ
	fspad_lcd_info->fix.smem_len = mWIDTH*mHEIGHT*2; 
	fspad_lcd_info->fix.type = FB_TYPE_PACKED_PIXELS;
	fspad_lcd_info->fix.visual = FB_VISUAL_TRUECOLOR;
	fspad_lcd_info->fix.line_length = mWIDTH*2;
	
	/*2.2���ÿɱ����*/
	fspad_lcd_info->var.xres = mWIDTH;
	fspad_lcd_info->var.yres = mHEIGHT;
	fspad_lcd_info->var.xres_virtual= mWIDTH;
	fspad_lcd_info->var.yres_virtual= mHEIGHT;
	fspad_lcd_info->var.xoffset= 0;
	fspad_lcd_info->var.yoffset= 0;
	fspad_lcd_info->var.bits_per_pixel = 16;
	//�Ҷ�Ĭ��Ϊ0
	fspad_lcd_info->var.grayscale = 0;
	fspad_lcd_info->var.red.offset = 11;
	fspad_lcd_info->var.red.length = 5;
	fspad_lcd_info->var.green.offset = 5;
	fspad_lcd_info->var.green.length = 6;
	fspad_lcd_info->var.blue.offset = 0;
	fspad_lcd_info->var.blue.length = 5;
	//������������
	fspad_lcd_info->var.activate = FB_ACTIVATE_NOW;

	/*2.3���ò�������*/
	fspad_lcd_info->fbops = &fspad_fb_ops;


	/*2.4������ص�����*/
	fspad_lcd_info->screen_size = fspad_lcd_info->fix.smem_len;
	
	//��ɫ��
	fspad_lcd_info->pseudo_palette = pseudo_palette ;


	/*3.Ӳ����صĲ���(����lcd������)*/
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
	
	//lcdʱ�ӳ�ʼ��
	*CLK_SRC_LCD0 &= ~0xf;
	*CLK_DIV_LCD &= ~0xf;
	*CLK_SRC_LCD0 |= 0x6;
	*LCDBLK_CFG |= (1<<1);
	printk("lcd clk is set done.\n");
	
	//GPIO�˿ڵĳ�ʼ��
	*GPF0CON = 0x22222222;
	*GPF1CON = 0x22222222;
	*GPF2CON = 0x22222222;
	*GPF3CON = 0x00222222;
	printk("lcd gpio is set done.\n");

	//����Ƴ�ʼ��
	*GPD0CON &= ~0xf<<4;
	*GPD0CON |= (0x1<<4);
	printk("lcd black led is set done.\n");
	
	//lcd�������ܽ�ӳ��
	//lcd������������
	lcd_controller_init();
	printk("lcd controler is set done.\n");

	/*4.ע��ṹ��*/
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
	/*�ͷ��ڴ�ռ�*/
	framebuffer_release(fspad_lcd_info);
	/*iounmap*/
	/*ע���ṹ��*/
	unregister_framebuffer(fspad_lcd_info);
	printk("fspad_lcd_exit.\n");
}

module_init(fspad_lcd_init);
module_exit(fspad_lcd_exit);
MODULE_LICENSE("GPL");


