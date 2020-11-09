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


//时钟验证
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

//分配缓冲区
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


/*定义结构体*/
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
		bit [31]:是否使能输入转子旁路(0不使能)
		bit [30-29]:选择输出图片格式(先选择RGB)
		bit [28-16]:目标图片水平大小TargetHsize 
		bit [15-14]:设置旋转的格式
		bit [13]:是否使能输出转子旁路(0不使能)
		bit [12-0]:目标图片垂直大小TargetVsize 
	*/
	*CITRGFMT0 = (3<<29)|(TargetHsize<<16)|(3<<14)|(TargetVsize<<0);

	printk(KERN_INFO "%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;

}
static int ov3640_reqbufs(struct file *file, void *fh, struct v4l2_requestbuffers *b)
{
	/*分配缓存*/
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

//开始摄像头采集
static int ov3640_streamon(struct file *file, void *fh, enum v4l2_buf_type i)
{
	printk(KERN_INFO "%s:%s:%d\n",__FILE__,__func__,__LINE__);
	/*
		bit [31]:设置为BT601
		bit [30]:设置偏移为0，0 (normally used)
		bit [29]:Reserved (Should be "0") 
		bit [28-16]:设置水平像素为640
		bit [15-14]:设置图片输出格式02
		bit [13-0]:设置图片的垂直像素480

	*/
	*CISRCFMT0 |= (TargetHsize<<16)|(0<<14)|(TargetHsize<<0);

	/*
		bit [31]:0关闭窗口功能，1开启窗口功能(关闭窗口采裁剪)
		bit [30/15-12]:清除溢出标志位
		bit [29]:清除行缓冲区溢出指示标志旋转
		bit [26-16]:垂直方向上侧裁剪大小(不裁剪)
		bit [11-0]:水平方向左侧裁剪大小(不裁剪)

	*/
	*CIWDOFST0 |= (1<<31);
	/*
		bit [31]:软件复位cam控制器
		bit [30]:Reserved (Should be set to 0) 
		bit [29]:0 = Selects ITU Camera B,1 = Selects ITU Camera A 
		bit [28-27]:用于选择信号源(设置为00)
		bit [26]:设置PCLK的极性0
		bit [25]:设置VSYNC极性1
		bit [24]:设置 HREF极性0
		bit [23]:保留位(不设置)
		bit [22]:溢出中断开关位(0不开溢出中断)
		bit [21]:行同步信号掩码(先设置为0)
		bit [20]:Reserved (Should be set to 1)
		bit [19]:清除中断标志位 IRQ_CLR(写1去清除中断)
		bit [18]:帧结束中断
		bit [17]:帧开始中断
		bit [16]:中断使能位
		bit [15-14]:保留(不设置)
		bit [13]:软件更新从何输入(DMA,camera input)
		bit [12]:窗口关闭
		bit [11]:保留(不设置)
		bit [10]:选择回写到A还是B
		bit [9]:Reserved (Should be set to "0") 
		bit [8]:选择是否生成jpeg格式的图片
		bit [7]:选择MIPI格式输入的摄像头
		bit [6]:指定写回输入信号
		bit [5]:颜色空间转换的方程
		bit [4]:仅在连接延迟计数交错模式和字段端口时使用此位
		bit [3]:选择ITU或者MIPI格式的摄像头
		bit [2]:是否使用交错模式
		bit [1]:field域的极性
		bit [0]:选择渐进的方法或者接口模式
	*/
	*CIGCTRL0 |= (1<<29)|(1<<25)|(1<<20)|(1<<19)|(1<<16);

	/*
		设置图片右侧裁剪的大小
		bit [31-28]:保留位
		bit [27-16]:水平方向右侧裁剪的大小(500)
		bit [15-12]:保留
		bit [11-0]:垂直方向下侧裁剪大小(360)
	*/
	*CIWDOFST20 =0;

	*CIOCTRL0 |= (1<<30); 
		
	//下面三个寄存器是和缩放相关的寄存器
	//预览通道缩放比例
	*CISCPRERATIOn = (10<<28)|(1<<16)|(1<<0);
	//设置预览通道的图片尺寸
	*CISCPREDSTn = (TargetHsize<<16)|(TargetVsize<<0);
	*CISCCTRLn &= ~((1<<28)|(1<<27)); 
	*CISCCTRLn |= (1<<30)|(1<<29)|(256<<16)|(256<<0);

	
	*CITAREAn = TargetHsize*TargetVsize;
	*ORGOSIZEn = (TargetVsize<<16)|(TargetHsize<<0);
	
	//所有的使能位

	/*
		31:用于使能摄像头控制器
		30:使能缩放
	*/

	*CIIMGCPTn |= (1<<31)|(1<<30);
	
	*CISCCTRLn |= (1<<15);//开始缩放

	printk("sn=%x, s2n=%x\n",*CISTATUSn,*CISTATUS2n);
	return 0;

}

//停止摄像头采集
static int ov3640_streamoff(struct file *file, void *fh, enum v4l2_buf_type i)
{
	printk(KERN_INFO "%s:%s:%d\n",__FILE__,__func__,__LINE__);
	*CISCCTRLn &= ~(1<<15);
	*CIIMGCPTn &= ~((1<<31)|(1<<30));

	return 0;

}

static const struct v4l2_ioctl_ops ov3640_ioctl_ops = {
	/*表示它是一个摄像头设备*/
	.vidioc_querycap 		= ov3640_querycap,
	
	/* 用于列举、获得、测试、设置摄像头的数据的格式 */
 	.vidioc_enum_fmt_vid_cap = ov3640_enum_fmt_vid_cap,
	.vidioc_g_fmt_vid_cap   = ov3640_g_fmt_vid_cap,
	.vidioc_try_fmt_vid_cap = ov3640_try_fmt_vid_cap,
	.vidioc_s_fmt_vid_cap   = ov3640_s_fmt_vid_cap,
	
	/* 缓冲区操作: 申请/查询/放入队列/取出队列 */
	.vidioc_reqbufs 	    = ov3640_reqbufs,

	/*通过读的方式来读取*/
#if 0
	.vidioc_querybuf 		= ov3640_querybuf,
	.vidioc_qbuf 			= ov3640_qbuf,
	.vidioc_dqbuf 	 		= ov3640_dqbuf,
#endif

	/*启动/停止*/
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

/*应用程序通过读的方式读取摄像头的数据*/
static ssize_t ov3640_fops_read(struct file *filp, char __user *buf, size_t size, loff_t *pos)
{
	int i,ret;
	printk(KERN_INFO "%s:%s:%d\n",__FILE__,__func__,__LINE__);

	wait_event_interruptible(ov3640_wq,condition);
	printk(KERN_INFO "read starting..........\n");

#if 0
	for(i=0; i<4; i++){
		ret = copy_to_user(buf,(void *)(buf_addr[i].virt_base),buf_size);
		if(ret){
			printk(KERN_INFO "copy_to_user is fail. ret = %d\n",ret);
			return -EFAULT;
		}
	
	}
#endif

	condition = 0;

	return buf_size;
}

int ov3640_mmap(struct file *filp, struct vm_area_struct *vma)
{
    u32 size = vma->vm_end - vma->vm_start;
    u32 pfn;

    vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

    /*                                                                                                                
     * page frame number of the address for a source frame
     * to be stored at.
     */
    pfn = __phys_to_pfn(buf_addr[0].phy_base);

    if ((vma->vm_flags & VM_WRITE) && !(vma->vm_flags & VM_SHARED)) {
        printk("%s: writable mapping must be shared\n", __func__);
        return -EINVAL;
    }    

    if (remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot)) {
        printk("%s: mmap fail\n", __func__);
        return -EINVAL;
    }

    return 0;

}

static const struct v4l2_file_operations ov3640_fops = {
    .owner     		= THIS_MODULE,
    .open           = ov3640_fops_open,
    .release        = ov3640_fops_release,
    .read           = ov3640_fops_read,
    /* 起到中转作用 */
    .unlocked_ioctl = video_ioctl2, 
    .mmap           = ov3640_mmap,
};


/*
	说明:这个函数必须实现否则在insmod时会出错
*/
void ov3640_v4l2_release(struct video_device *vdev)
{
	int i;
	//释放缓存
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
			.flags = 0,          //表示写数据
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
	//为了使信号稳定是能上拉电阻
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
		GPL0CON[1]:CAM2M_RST;   //复位管脚先置为0
		GPL0CON[1]:CAM2M_PWDN;  //默认一直为低电平
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
	/*复位摄像头控制器*/
	//传输方式为BT601
	*CISRCFMT0 |= (1<<31);

	//复位camera interface

	*CIGCTRL0  |= (1<<31);
	mdelay(10);
	*CIGCTRL0  &= ~(1<<31);
	mdelay(10);

	//对寄存器复位
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
	struct clk *cam0_clk,*fimc0_clk,*mout_fimc0,*mout_mpll,*fimc0;
/*************************设置外部摄像头时钟为24MHz*********************************/
	/*读取cam0默认时钟1.5MHZ*/
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
	
	
/*****************************设置控制器的父节点*******************************/
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
	if(!ret){
		printk("mout_fimc0 set parent mout_mpll_user_t is ok.\n");
		}
	printk("mout_fimc0 = %ld\n",clk_get_rate(mout_fimc0));
	

/***************************设置控制器的时钟为160MHz*******************************/
	fimc0_clk = __clk_lookup("sclk_fimc0");
	if (!fimc0_clk) {
		printk("%s: could not find clock %s\n", __func__, "fimc0_clk");
		return ;
	}
	clk_prepare(fimc0_clk);
	clk_set_rate(fimc0_clk,160000000); //暂时设置为160MHZ
	clk_enable(fimc0_clk);
	printk("sclk_fimc0 = %ld\n",_get_rate("sclk_fimc0"));
	

/***************************设置fimc0的总线速率*******************************/
	fimc0 = __clk_lookup("fimc0");
	if (!fimc0) {
		printk("%s: could not find clock %s\n", __func__, "fimc0");
		return ;
	}
	clk_prepare(fimc0);
	clk_enable(fimc0);
	printk("fimc0 = %ld\n",clk_get_rate(fimc0));

/**************************等待时钟稳定********************************/
	//延时下等待时钟稳定
	mdelay(20);

	//为了检测时钟是否设置进去了
	printk("clk_src_cam0 = %x\n",*CLK_SRC_CAM0);
	printk("clk_div_cam0 = %x\n",*CLK_DIV_CAM0);

}

static void ov3640_reset(void)
{
	/*
		GPL0CON[1]:CAM2M_RST;   //复位1---->0---->1
		GPL0CON[3]:CAM2M_PWDN;  //默认一直为低电平(不是这一位啊啊!!)
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
	/*读取id*/
	mid = read_reg(client,0x300a);
	printk("mid = %x\n",mid);
	mid = read_reg(client,0x300b);
	printk("mid = %x\n",mid);

	/*通过i2c对数据格式进行设置*/
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

static irqreturn_t ov3640_fimc_interrupt(int irq, void *dev_id)
{
	//清除中断
	clear_interrupt_flag();
	
	//唤醒等待队列
	wake_up_interruptible(&ov3640_wq);
	
	//设置中断条件为真
	condition = 1;

	return IRQ_HANDLED;
}

static int ov3640_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct device_node *mynode;
	
	printk(KERN_INFO "%s:%s:%d\n",__FILE__,__func__,__LINE__);
		
	/*初始化结构体*/
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

/***************************************************************/
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

	
	/*硬件相关的操作*/
	//这三个寄存器是用来设置camera硬件接口的
	/*硬件相关的操作*/
	GPJ0CON = ioremap(0x11400240,4);
	GPJ0DAT = ioremap(0x11400244,4);
	GPJ0PUD = ioremap(0x11400248,4);

	GPJ1CON = ioremap(0x11400260,4);
	GPJ1DAT = ioremap(0x11400264,4);
	GPJ1PUD = ioremap(0x11400268,4);

	GPL0CON = ioremap(0x110000c0,4);
	GPL0DAT = ioremap(0x110000c4,4);
	GPL0PUD = ioremap(0x110000c8,4);


	//用于判断时钟是否设置成功
	CLK_SRC_CAM0 = ioremap(0x1003C220,4);
	CLK_DIV_CAM0 = ioremap(0x1003C520,4);

	//camera interface寄存器映射
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

	//对摄像头的接口进行管脚初始化
	cam_gpio_init();

	//设置摄像头时钟
	cam_clk_init();

	//复位摄像头camif控制器
	camif_reset();

	//复位摄像头模块
	ov3640_reset();
		
	//使用i2c总线，初始化摄像头
	ov3640_init(client);
	
/***************************************************************/
	init_waitqueue_head(&ov3640_wq);
	mynode = of_find_node_by_path("/camera/fimc@11800000");
	if(mynode == NULL)
	{
		printk("get node is fail.please try again\n");
		return -EAGAIN;
	}

	irq_num = irq_of_parse_and_map(mynode,0);
	printk("fimc irq num : %d\n",irq_num);
	
	ret = request_irq(irq_num,ov3640_fimc_interrupt,IRQF_DISABLED,"ov3640_interrupt",NULL);
	if(ret){
		printk("register irq is fail.\n");
		return -EAGAIN;
	}
	
/**************************************************************************/

	/*结构体的注册*/
	/*VFL_TYPE_GRABBER表示的是摄像头设备，-1表示自动分配此设备号*/
	if (video_register_device(ov3640_vd,VFL_TYPE_GRABBER,-1) < 0) {
		printk("video_register_device failed\n");
		return -ENODEV;
	}	
	
	return 0;
}
static int ov3640_remove(struct i2c_client *client)
{
	printk(KERN_INFO "%s:%s:%d\n",__FILE__,__func__,__LINE__);

	free_irq(irq_num,NULL);
	
	/*结构体的注销*/
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

