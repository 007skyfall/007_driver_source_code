#include <linux/module.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#include <asm/page.h>
#include <linux/font.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/of_irq.h>
#include <linux/videodev2.h>
#include <linux/freezer.h>
#include <media/videobuf2-vmalloc.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-event.h>                                                   
#include <media/v4l2-common.h>
#include <linux/clk-private.h>

#include "ov3640.h"
#include "clk.h"

//camera gpio
static unsigned int * GPJ0CON;
static unsigned int * GPJ0DAT;
static unsigned int * GPJ0PUD;

static unsigned int * GPJ1CON;
static unsigned int * GPJ1DAT;
static unsigned int * GPJ1PUD;

static unsigned int * GPL0CON;
static unsigned int * GPL0DAT;
static unsigned int * GPL0PUD;


//ʱ����֤
static unsigned int * CLK_SRC_CAM0;
static unsigned int * CLK_DIV_CAM0;

//cameraif
static unsigned int * BASE_REGS;
static unsigned int * CISRCFMT0;
static unsigned int * CIGCTRL0;
static unsigned int * CIWDOFST0;
static unsigned int * CIWDOFST20;
static unsigned int * CITRGFMT0;
static unsigned int * CIOCTRL0;

//���仺����
static unsigned int * CIOYSA10;
static unsigned int * CIOYSA20;
static unsigned int * CIOYSA30;
static unsigned int * CIOYSA40;

static unsigned int *CISCPRERATIOn;
static unsigned int *CISCPREDSTn;
static unsigned int *CISCCTRLn;
static unsigned int *CITAREAn;
static unsigned int *ORGOSIZEn;
static unsigned int *CIIMGCPTn;
static unsigned int *CISTATUSn;
static unsigned int *CISTATUS2n;

static unsigned int buf_size;
static unsigned int TargetHsize = 640;
static unsigned int TargetVsize = 480;


struct camif_mem_addr {
	unsigned int virt_base;
	unsigned int phy_base;
};

struct ov3640_fmt {
	const char *name;
	u32   fourcc;          /* v4l2 format id */
	u8    depth;
	bool  is_yuv;
};

wait_queue_head_t ov3640_wq;
int condition = 0;
int irq_num;
unsigned int order;

static const struct ov3640_fmt formats[] = {
	{
		.name     = "RGB565",
		.fourcc   = V4L2_PIX_FMT_RGB565,
		.depth    = 16,
	},
	{
		.name     = "RGB888",
		.fourcc   = V4L2_PIX_FMT_RGB24,
		.depth    = 24,
	},
};

struct camif_mem_addr buf_addr[] = {
	{
		.virt_base = (unsigned int)NULL,
		.phy_base = (unsigned int)NULL,
	},
	{
		.virt_base = (unsigned int)NULL,
		.phy_base = (unsigned int)NULL,
	},
	{
		.virt_base = (unsigned int)NULL,
		.phy_base = (unsigned int)NULL,
	},
	{
		.virt_base = (unsigned int)NULL,
		.phy_base = (unsigned int)NULL,
	}
};


/*����ṹ��*/
static struct video_device *ov3640_vd = NULL;

static int ov3640_querycap(struct file *file, void *fh, struct v4l2_capability *cap)
{
	memset(cap,0,sizeof(*cap));
	strcpy(cap->driver, "ov3640");
	strcpy(cap->card, "ov3640");
	cap->version = 1;
	
	cap->capabilities = V4L2_CAP_VIDEO_CAPTURE| V4L2_CAP_READWRITE;

	printk(KERN_INFO "%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}

static int ov3640_enum_fmt_vid_cap(struct file *file, void *fh,struct v4l2_fmtdesc *f)
{
	const struct ov3640_fmt *fmt;

	if (f->index >= ARRAY_SIZE(formats))
		return -EINVAL;

	fmt = &formats[f->index];

	strlcpy(f->description, fmt->name, sizeof(f->description));
	f->pixelformat = fmt->fourcc;
	
	printk(KERN_INFO "%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}

static int ov3640_g_fmt_vid_cap(struct file *file, void *fh,struct v4l2_format *f)
{
	printk(KERN_INFO "%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;

}

static int ov3640_try_fmt_vid_cap(struct file *file, void *fh,struct v4l2_format *f)
{
	if(f->type != V4L2_BUF_TYPE_VIDEO_CAPTURE){
		return -EINVAL;
	}

	if((f->fmt.pix.pixelformat != V4L2_PIX_FMT_RGB24)&&\
		(f->fmt.pix.pixelformat != V4L2_PIX_FMT_RGB565)){
		return -EINVAL;
	}

	printk(KERN_INFO "%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;

}

static int ov3640_s_fmt_vid_cap(struct file *file, void *fh,struct v4l2_format *f)
{
	int ret;
	ret = ov3640_try_fmt_vid_cap(file, NULL,f);
	if (ret < 0){
		return ret;
	}
	if((f->fmt.pix.width <= 0) && (f->fmt.pix.height <= 0)){
			 f->fmt.pix.width = TargetHsize;
			 f->fmt.pix.height = TargetVsize;
		}
	if(f->fmt.pix.pixelformat == V4L2_PIX_FMT_RGB565){
		f->fmt.pix.bytesperline = (f->fmt.pix.width * 16)>>3;
		f->fmt.pix.sizeimage = f->fmt.pix.height * f->fmt.pix.bytesperline;
		buf_size = f->fmt.pix.sizeimage;

		//RGB565
		*CISCCTRLn &= ~(3<<11);
	}else if(f->fmt.pix.pixelformat == V4L2_PIX_FMT_RGB24){
		f->fmt.pix.bytesperline = (f->fmt.pix.width * 24)>>3;
		f->fmt.pix.sizeimage = f->fmt.pix.height * f->fmt.pix.bytesperline;
		buf_size = f->fmt.pix.sizeimage;

		//RGB888
		*CISCCTRLn &= ~(3<<11);
		*CISCCTRLn |= (2<<11);

	}
	/*
		bit [31]:�Ƿ�ʹ������ת����·(0��ʹ��)
		bit [30-29]:ѡ�����ͼƬ��ʽ(��ѡ��RGB)
		bit [28-16]:Ŀ��ͼƬˮƽ��СTargetHsize 
		bit [15-14]:������ת�ĸ�ʽ
		bit [13]:�Ƿ�ʹ�����ת����·(0��ʹ��)
		bit [12-0]:Ŀ��ͼƬ��ֱ��СTargetVsize 
	*/
	*CITRGFMT0 = (3<<29)|(TargetHsize<<16)|(3<<14)|(TargetVsize<<0);

	printk(KERN_INFO "%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;

}
static int ov3640_reqbufs(struct file *file, void *fh, struct v4l2_requestbuffers *b)
{
	/*���仺��*/
	int i;
	order = get_order(buf_size);
	printk("buffer size : %d\n",buf_size);
	for(i=0;i<4;i++){
		buf_addr[i].virt_base= __get_free_pages( GFP_KERNEL|GFP_DMA,order);
		if(buf_addr[i].virt_base == (unsigned int)NULL)
		{
			printk("get_free_pages is fail\n");
		}
		buf_addr[i].phy_base = __virt_to_phys(buf_addr[i].virt_base);
	}
	
	*CIOYSA10 = buf_addr[0].phy_base;
	*CIOYSA20 = buf_addr[1].phy_base;
	*CIOYSA30 = buf_addr[2].phy_base;
	*CIOYSA40 = buf_addr[3].phy_base;

	printk(KERN_INFO "%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}

//��ʼ����ͷ�ɼ�
static int ov3640_streamon(struct file *file, void *fh, enum v4l2_buf_type i)
{
	printk(KERN_INFO "%s:%s:%d\n",__FILE__,__func__,__LINE__);
	/*
		bit [31]:����ΪBT601
		bit [30]:����ƫ��Ϊ0��0 (normally used)
		bit [29]:Reserved (Should be "0") 
		bit [28-16]:����ˮƽ����Ϊ640
		bit [15-14]:����ͼƬ�����ʽ02
		bit [13-0]:����ͼƬ�Ĵ�ֱ����480

	*/
	*CISRCFMT0 |= (TargetHsize<<16)|(0<<14)|(TargetHsize<<0);

	/*
		bit [31]:0�رմ��ڹ��ܣ�1�������ڹ���(�رմ��ڲɲü�)
		bit [30/15-12]:��������־λ
		bit [29]:����л��������ָʾ��־��ת
		bit [26-16]:��ֱ�����ϲ�ü���С(���ü�)
		bit [11-0]:ˮƽ�������ü���С(���ü�)

	*/
	*CIWDOFST0 |= (1<<31);
	/*
		bit [31]:�����λcam������
		bit [30]:Reserved (Should be set to 0) 
		bit [29]:0 = Selects ITU Camera B,1 = Selects ITU Camera A 
		bit [28-27]:����ѡ���ź�Դ(����Ϊ00)
		bit [26]:����PCLK�ļ���0
		bit [25]:����VSYNC����1
		bit [24]:���� HREF����0
		bit [23]:����λ(������)
		bit [22]:����жϿ���λ(0��������ж�)
		bit [21]:��ͬ���ź�����(������Ϊ0)
		bit [20]:Reserved (Should be set to 1)
		bit [19]:����жϱ�־λ IRQ_CLR(д1ȥ����ж�)
		bit [18]:֡�����ж�
		bit [17]:֡��ʼ�ж�
		bit [16]:�ж�ʹ��λ
		bit [15-14]:����(������)
		bit [13]:������´Ӻ�����(DMA,camera input)
		bit [12]:���ڹر�
		bit [11]:����(������)
		bit [10]:ѡ���д��A����B
		bit [9]:Reserved (Should be set to "0") 
		bit [8]:ѡ���Ƿ�����jpeg��ʽ��ͼƬ
		bit [7]:ѡ��MIPI��ʽ���������ͷ
		bit [6]:ָ��д�������ź�
		bit [5]:��ɫ�ռ�ת���ķ���
		bit [4]:���������ӳټ�������ģʽ���ֶζ˿�ʱʹ�ô�λ
		bit [3]:ѡ��ITU����MIPI��ʽ������ͷ
		bit [2]:�Ƿ�ʹ�ý���ģʽ
		bit [1]:field��ļ���
		bit [0]:ѡ�񽥽��ķ������߽ӿ�ģʽ
	*/
	*CIGCTRL0 |= (1<<29)|(1<<25)|(1<<20)|(1<<19)|(1<<16);

	/*
		����ͼƬ�Ҳ�ü��Ĵ�С
		bit [31-28]:����λ
		bit [27-16]:ˮƽ�����Ҳ�ü��Ĵ�С(500)
		bit [15-12]:����
		bit [11-0]:��ֱ�����²�ü���С(360)
	*/
	*CIWDOFST20 =0;

	*CIOCTRL0 |= (1<<30); 
		
	//���������Ĵ����Ǻ�������صļĴ���
	//Ԥ��ͨ�����ű���
	*CISCPRERATIOn = (10<<28)|(1<<16)|(1<<0);
	//����Ԥ��ͨ����ͼƬ�ߴ�
	*CISCPREDSTn = (TargetHsize<<16)|(TargetVsize<<0);
	*CISCCTRLn &= ~((1<<28)|(1<<27));  
	*CISCCTRLn |= (1<<30)|(1<<29)|(256<<16)|(256<<0);

	
	*CITAREAn = TargetHsize*TargetVsize;
	*ORGOSIZEn = (TargetVsize<<16)|(TargetHsize<<0);
	
	//���е�ʹ��λ

	/*
		31:����ʹ������ͷ������
		30:ʹ������
	*/

	*CIIMGCPTn |= (1<<31)|(1<<30);
	
	*CISCCTRLn |= (1<<15);//��ʼ����

	printk("sn=%x, s2n=%x\n",*CISTATUSn,*CISTATUS2n);
	return 0;

}

//ֹͣ����ͷ�ɼ�
static int ov3640_streamoff(struct file *file, void *fh, enum v4l2_buf_type i)
{
	printk(KERN_INFO "%s:%s:%d\n",__FILE__,__func__,__LINE__);
//	*CISCCTRLn &= ~(1<<15);
//	*CIIMGCPTn &= ~((1<<31)|(1<<30));

	return 0;

}

static const struct v4l2_ioctl_ops ov3640_ioctl_ops = {
	/*��ʾ����һ������ͷ�豸*/
	.vidioc_querycap 		= ov3640_querycap,
	
	/* �����о١���á����ԡ���������ͷ�����ݵĸ�ʽ */
 	.vidioc_enum_fmt_vid_cap = ov3640_enum_fmt_vid_cap,
	.vidioc_g_fmt_vid_cap   = ov3640_g_fmt_vid_cap,
	.vidioc_try_fmt_vid_cap = ov3640_try_fmt_vid_cap,
	.vidioc_s_fmt_vid_cap   = ov3640_s_fmt_vid_cap,
	
	/* ����������: ����/��ѯ/�������/ȡ������ */
	.vidioc_reqbufs 	    = ov3640_reqbufs,

	/*ͨ�����ķ�ʽ����ȡ*/
#if 0
	.vidioc_querybuf 		= ov3640_querybuf,
	.vidioc_qbuf 			= ov3640_qbuf,
	.vidioc_dqbuf 	 		= ov3640_dqbuf,
#endif

	/*����/ֹͣ*/
	.vidioc_streamon 		= ov3640_streamon,
	.vidioc_streamoff 		= ov3640_streamoff,

};

static int ov3640_fops_open(struct file *filp)
{
	printk(KERN_INFO "%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}
static int ov3640_fops_release(struct file *filp)
{
	printk(KERN_INFO "%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}

/*Ӧ�ó���ͨ�����ķ�ʽ��ȡ����ͷ������*/
static ssize_t ov3640_fops_read(struct file *filp, char __user *buf, size_t size, loff_t *pos)
{
	int i,ret;
	printk(KERN_INFO "%s:%s:%d\n",__FILE__,__func__,__LINE__);

	//wait_event_interruptible(ov3640_wq,condition);
	mdelay(500);
	printk(KERN_INFO "read starting..........\n");
	//for(i=0; i<4; i++){
		ret = copy_to_user(buf,(void *)(buf_addr[1].virt_base),buf_size);
		if(ret){
			printk(KERN_INFO "copy_to_user is fail. ret = %d\n",ret);
			return -EFAULT;
		}
	
	//}

	condition = 0;

	return buf_size;
}

static const struct v4l2_file_operations ov3640_fops = {
    .owner     		= THIS_MODULE,
    .open           = ov3640_fops_open,
    .release        = ov3640_fops_release,
    .read           = ov3640_fops_read,
    /* ����ת���� */
    .unlocked_ioctl = video_ioctl2, 
};


/*
	˵��:�����������ʵ�ַ�����insmodʱ�����
*/
void ov3640_v4l2_release(struct video_device *vdev)
{
	int i;
	//�ͷŻ���
	for(i=0; i<4; i++){
		free_pages(buf_addr[i].virt_base,order);
	}
}


static int read_reg(struct i2c_client *client,int reg)
{
	unsigned char reg_h = reg >> 8;
	unsigned char reg_l = (reg & 0xff);
	unsigned char txbuf[2] = { reg_h,reg_l };  
    unsigned char rxbuf[1];  
	struct i2c_msg msgs[2] = {
		[0] = {
			.addr = client->addr,
			.flags = 0,         
			.len = 2,
			.buf = txbuf,
		},
		[1] = {
			.addr = client->addr,
			.flags = I2C_M_RD,          
			.len = sizeof(rxbuf),
			.buf = rxbuf,
		},
	};
	
	i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
	
	return rxbuf[0];
}

static int write_reg(struct i2c_client *client,int reg,int val)
{
	unsigned char reg_h = reg >> 8;
	unsigned char reg_l = (reg & 0xff);
	unsigned char wbuf[3] = {reg_h,reg_l,val};
	struct i2c_msg msgs[1] = {
		[0] = {
			.addr = client->addr,
			.flags = 0,          //��ʾд����
			.len = 3,
			.buf = wbuf,
			}
		};
	
	i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
	
	return 0;
}

void cam_gpio_init(void)
{
	/*
		GPJ0CON[31:28] : CAM_A_DATA[4] 
		GPJ0CON[27:24] : CAM_A_DATA[3] 
		GPJ0CON[23:20] : CAM_A_DATA[2] 
		GPJ0CON[19:16] : CAM_A_DATA[1] 
		GPJ0CON[15:12] : CAM_A_DATA[0]
		GPJ0CON[11:8]  : CAM_A_HREF 
		GPJ0CON[7:4]   : CAM_A_VSYNC 
		GPJ0CON[3:0]   : CAM_A_PCLK   
	*/
	*GPJ0CON = 0x22222222;
	*GPJ0DAT = 0;	
	//Ϊ��ʹ�ź��ȶ�������������
	*GPJ0PUD = 0xffff;
	/*
		GPJ1CON[19:16] : CAM_A_FIELD  
		GPJ1CON[15:12] : CAM_A_CLKOUT 
		GPJ1CON[11:8]  : CAM_A_DATA[7] 
		GPJ1CON[7:4]   : CAM_A_DATA[6]
		GPJ1CON[3:0]   : CAM_A_DATA[5]  
	*/
	*GPJ1CON = 0x22222;
	*GPJ1DAT = 0;
	*GPJ1PUD = 0x3ff;

	/*
		GPL0CON[1]:CAM2M_RST;   //��λ�ܽ�����Ϊ0
		GPL0CON[1]:CAM2M_PWDN;  //Ĭ��һֱΪ�͵�ƽ
	*/
	*GPL0CON |= (1<<12)|(1<<4); 
	*GPL0DAT &= ~(1<<3);
	*GPL0DAT |= (1<<1);
	*GPL0PUD |= ((0x3<<2)|(0x3<<6));

}


static void fimc_reset_cfg(void)
{
	int i;
	u32 cfg[][2] = {
		{ 0x018, 0x00000000 }, { 0x01c, 0x00000000 },
		{ 0x020, 0x00000000 }, { 0x024, 0x00000000 },
		{ 0x028, 0x00000000 }, { 0x02c, 0x00000000 },
		{ 0x030, 0x00000000 }, { 0x034, 0x00000000 },
		{ 0x038, 0x00000000 }, { 0x03c, 0x00000000 },
		{ 0x040, 0x00000000 }, { 0x044, 0x00000000 },
		{ 0x048, 0x00000000 }, { 0x04c, 0x00000000 },
		{ 0x050, 0x00000000 }, { 0x054, 0x00000000 },
		{ 0x058, 0x18000000 }, { 0x05c, 0x00000000 },
		{ 0x064, 0x00000000 },
		{ 0x0c0, 0x00000000 }, { 0x0c4, 0xffffffff },
		{ 0x0d0, 0x00100080 }, { 0x0d4, 0x00000000 },
		{ 0x0d8, 0x00000000 }, { 0x0dc, 0x00000000 },
		{ 0x0f8, 0x00000000 }, { 0x0fc, 0x04000000 },
		{ 0x168, 0x00000000 }, { 0x16c, 0x00000000 },
		{ 0x170, 0x00000000 }, { 0x174, 0x00000000 },
		{ 0x178, 0x00000000 }, { 0x17c, 0x00000000 },
		{ 0x180, 0x00000000 }, { 0x184, 0x00000000 },
		{ 0x188, 0x00000000 }, { 0x18c, 0x00000000 },
		{ 0x194, 0x0000001e },
	};

	for (i = 0; i < sizeof(cfg) / 8; i++)
		writel(cfg[i][1], BASE_REGS + cfg[i][0]);
}

void camif_reset(void)
{
	/*��λ����ͷ������*/
	//���䷽ʽΪBT601
	*CISRCFMT0 |= (1<<31);

	//��λcamera interface

	*CIGCTRL0  |= (1<<31);
	mdelay(10);
	*CIGCTRL0  &= ~(1<<31);
	mdelay(10);

	//�ԼĴ�����λ
	fimc_reset_cfg();
}

void cam_clk_init(void)
{
	int ret;
	/*
		1. System bus clock   
		2. Camera pixel clock, PCLK   
		3. Internal core clock 
	*/
	struct clk *cam0_clk,*fimc0_clk,*mout_fimc0,*mout_mpll;
/*************************�����ⲿ����ͷʱ��Ϊ24MHz*********************************/
	/*��ȡcam0Ĭ��ʱ��1.5MHZ*/
	cam0_clk = __clk_lookup("sclk_cam0");
	if (!cam0_clk) {
		printk("%s: could not find clock %s\n", __func__, "sclk_cam0");
		return ;
	}
	printk("sclk_cam0 = %ld\n",clk_get_rate(cam0_clk));
	clk_prepare(cam0_clk);
	clk_set_rate(cam0_clk,24000000);
	clk_enable(cam0_clk);
	printk("sclk_cam0 = %ld\n",_get_rate("sclk_cam0"));


	
/*****************************���ÿ������ĸ��ڵ�*******************************/
	mout_mpll = __clk_lookup("mout_mpll_user_t");
	if (!mout_mpll) {
		printk("%s: could not find clock %s\n", __func__, "mout_mpll");
		return ;
	}
	printk("mout_mpll = %ld\n",clk_get_rate(mout_mpll));
	

	mout_fimc0 = __clk_lookup("mout_fimc0");
	if (!mout_fimc0) {
		printk("%s: could not find clock %s\n", __func__, "mout_fimc0");
		return ;
	}
	ret = clk_set_parent(mout_fimc0,mout_mpll);
	printk("clk set parent is ret = %d\n",ret);
	printk("mout_fimc0 = %ld\n",clk_get_rate(mout_fimc0));
	

/***************************���ÿ�������ʱ��Ϊ160MHz*******************************/
	fimc0_clk = __clk_lookup("sclk_fimc0");
	if (!fimc0_clk) {
		printk("%s: could not find clock %s\n", __func__, "fimc0_clk");
		return ;
	}
	clk_prepare(fimc0_clk);
	clk_set_rate(fimc0_clk,160000000); //��ʱ����Ϊ160MHZ
	clk_enable(fimc0_clk);
	printk("sclk_fimc0 = %ld\n",_get_rate("sclk_fimc0"));

/**************************�ȴ�ʱ���ȶ�********************************/
	//��ʱ�µȴ�ʱ���ȶ�
	mdelay(20);

	//Ϊ�˼��ʱ���Ƿ����ý�ȥ��
	printk("clk_src_cam0 = %x\n",*CLK_SRC_CAM0);
	printk("clk_div_cam0 = %x\n",*CLK_DIV_CAM0);

}

static void ov3640_reset(void)
{
	/*
		GPL0CON[1]:CAM2M_RST;   //��λ1---->0---->1
		GPL0CON[3]:CAM2M_PWDN;  //Ĭ��һֱΪ�͵�ƽ(������һλ����!!)
	*/
	*GPL0DAT |= (1<<1);
	mdelay(30);
	*GPL0DAT &= ~(1<<1);
	mdelay(30);
	*GPL0DAT |= (1<<1);
	mdelay(30);
}


void ov3640_init(struct i2c_client *client)
{
	unsigned int mid;
	int i=0;
	/*��ȡid*/
	mid = read_reg(client,0x300a);
	printk("mid = %x\n",mid);
	mid = read_reg(client,0x300b);
	printk("mid = %x\n",mid);

	/*ͨ��i2c�����ݸ�ʽ��������*/
	//YUYV422   8BIT   QXGA(1600*1200)
	for(i=0;i<sizeof(ov3640_i2c_regs)/sizeof(ov3640_i2c_regs[0]);i++)
	{
		write_reg(client,ov3640_i2c_regs[i][0],ov3640_i2c_regs[i][1]);
		mdelay(2);
	}

}

void clear_interrupt_flag(void)
{
	*CIGCTRL0 |= (1<<19);  
}

#if 1
static irqreturn_t ov3640_fimc_interrupt(int irq, void *dev_id)
{
	clear_interrupt_flag();
	
	printk("this is interrupt func.!!!!!!!!!!!!!!!!!!!!!!!!\n");

	//���ѵȴ�����
	wake_up_interruptible(&ov3640_wq);
	
	//�����ж�����Ϊ��
	condition = 1;

	return IRQ_HANDLED;
}
#endif

static int ov3640_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;

	struct device_node *mynode;
		
	printk(KERN_INFO "%s:%s:%d\n",__FILE__,__func__,__LINE__);
	/*��ʼ���ṹ��*/
	ov3640_vd =  video_device_alloc();
	if(ov3640_vd == NULL)
	{
		printk("alloc memory is fail\n");
		return -ENOMEM;
	}
	ov3640_vd->v4l2_dev = kzalloc(sizeof(struct v4l2_device), GFP_KERNEL);
	if(ov3640_vd->v4l2_dev == NULL)
	{
		printk("alloc memory is fail\n");
		return -ENOMEM;
	}

	strcpy(ov3640_vd->v4l2_dev->name,"ov3640");
	ret = v4l2_device_register(NULL, ov3640_vd->v4l2_dev);
	if(ret){
		printk("v4l2 init is fail\n");
		return -EAGAIN;
	}
	
	strcpy(ov3640_vd->name,"ov3640");
	ov3640_vd->fops = &ov3640_fops;
	ov3640_vd->ioctl_ops = &ov3640_ioctl_ops;
	ov3640_vd->release = ov3640_v4l2_release;
	
	/*Ӳ����صĲ���*/
	//�������Ĵ�������������cameraӲ���ӿڵ�
	/*Ӳ����صĲ���*/
	GPJ0CON = ioremap(0x11400240,4);
	GPJ0DAT = ioremap(0x11400244,4);
	GPJ0PUD = ioremap(0x11400248,4);

	GPJ1CON = ioremap(0x11400260,4);
	GPJ1DAT = ioremap(0x11400264,4);
	GPJ1PUD = ioremap(0x11400268,4);

	GPL0CON = ioremap(0x110000c0,4);
	GPL0DAT = ioremap(0x110000c4,4);
	GPL0PUD = ioremap(0x110000c8,4);


	//�����ж�ʱ���Ƿ����óɹ�
	CLK_SRC_CAM0 = ioremap(0x1003C220,4);
	CLK_DIV_CAM0 = ioremap(0x1003C520,4);

	//camera interface�Ĵ���ӳ��
	BASE_REGS = CISRCFMT0 = ioremap(0x11800000,4);
	CIGCTRL0 = ioremap(0x11800008,4);
	CIWDOFST0 = ioremap(0x11800004,4);
	CIWDOFST20 = ioremap(0x11800014,4);

	CIOYSA10 = ioremap(0x11800018,4);
	CIOYSA20 = ioremap(0x1180001c,4);
	CIOYSA30 = ioremap(0x11800020,4);
	CIOYSA40 = ioremap(0x11800024,4);

	CITRGFMT0 = ioremap(0x11800048,4);
	CIOCTRL0 = ioremap(0x1180004c,4);

	CISCPRERATIOn = ioremap(0x11800050,4);
	CISCPREDSTn = ioremap(0x11800054,4);
	CISCCTRLn = ioremap(0x11800058,4);
	CITAREAn = ioremap(0x1180005c,4);
	ORGOSIZEn = ioremap(0x11800184,4);
	CIIMGCPTn = ioremap(0x118000c0,4); 
	CISTATUSn = ioremap(0x11800064,4); 
	CISTATUS2n = ioremap(0x11800068,4); 

	//������ͷ�Ľӿڽ��йܽų�ʼ��
	cam_gpio_init();

	//��������ͷʱ��
	cam_clk_init();

	//��λ����ͷcamif������
	camif_reset();

	//��λ����ͷģ��
	ov3640_reset();
		
	//ʹ��i2c���ߣ���ʼ������ͷ
	ov3640_init(client);


	/*�ṹ���ע��*/
	/*VFL_TYPE_GRABBER��ʾ��������ͷ�豸��-1��ʾ�Զ�������豸��*/
	if (video_register_device(ov3640_vd,VFL_TYPE_GRABBER,-1) < 0) {
		printk("video_register_device failed\n");
		return -ENODEV;
	}	
	
	init_waitqueue_head(&ov3640_wq);
	mynode = of_find_node_by_path("/camera/fimc@11800000");
	if(mynode == NULL)
	{
		printk("get node is fail.please try again\n");
		return -EAGAIN;
	}

	irq_num = irq_of_parse_and_map(mynode,0);
	irq_num = 148;
	printk("fimc irq num : %d\n",irq_num);
	
	ret = request_irq(irq_num,ov3640_fimc_interrupt,IRQF_DISABLED,"ov3640_interrupt",NULL);
	if(ret){
		printk("register irq is fail.\n");
		return -EAGAIN;
	}
	
	return 0;
}
static int ov3640_remove(struct i2c_client *client)
{
	printk(KERN_INFO "%s:%s:%d\n",__FILE__,__func__,__LINE__);

	free_irq(irq_num,NULL);
	
	/*�ṹ���ע��*/
	video_unregister_device(ov3640_vd);
	return 0;
}

static const struct of_device_id ov3640_of_ids[] = {
	{ .compatible = "ov3640" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, ov3640_of_ids);

static const struct i2c_device_id ov3640_id_ids[] = {
	{ "ov3640", 0x3c},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(i2c, ov3640_id_ids);


static struct i2c_driver ov3640_drv ={
	.probe  = ov3640_probe,
	.remove = ov3640_remove,
	.driver ={
		.name = "ov3640",
		.of_match_table = ov3640_of_ids,
	}, 
	.id_table = ov3640_id_ids,
};

module_i2c_driver(ov3640_drv);
MODULE_AUTHOR("dzs");
MODULE_LICENSE("GPL");

