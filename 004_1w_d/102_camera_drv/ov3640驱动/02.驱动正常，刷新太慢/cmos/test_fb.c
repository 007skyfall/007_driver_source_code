#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <string.h>

struct RGB_COMB {
	unsigned char RGB_h;
	unsigned char RGB_l;
};

#define LENGTH 614400
char image_buf[LENGTH] = {0};
int fbfd;
int ret;
int x,y,i=0;
unsigned int screen_size;
char *  virtual_addr;
struct fb_var_screeninfo varinfo;
struct RGB_COMB new_rgb;
unsigned short rgb;


int camera_device_open(void)
{
	int fd;
	//用阻塞模式打开摄像头设备
	fd = open("/dev/video0",O_RDWR,0);
	if(fd < 0){
		perror("open /dev/video0 is fail.\n");
		exit(EXIT_FAILURE);
	}
	return fd;
}

int init_camera_attribute(int fd)
{
	int numBufs;
	v4l2_std_id id;
	struct v4l2_format fmt;
	struct v4l2_requestbuffers  req;
	struct v4l2_buffer    buf;

	//检查当前视频设备支持的标准
	ioctl(fd,VIDIOC_QUERYSTD,&id);
	
	//设置视频捕获格式
	memset(&fmt,0,sizeof(fmt));
	fmt.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = 0;
	fmt.fmt.pix.height = 0;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;

	if(ioctl(fd,VIDIOC_S_FMT,&fmt) == -1){
		perror("set VIDIOC_S_FMT is fail");
		exit(EXIT_FAILURE);
	}
	
	//分配内存
	memset(&req,0,sizeof(req));
	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	
	if(ioctl(fd,VIDIOC_REQBUFS,&req) == -1){
		perror("set VIDIOC_REQBUFS is fail");
		exit(EXIT_FAILURE);
	}

	return 0;
}

int start_capturing(int fd)
{
	enum v4l2_buf_type type;

	//开始采集数据
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(fd,VIDIOC_STREAMON,&type) == -1){
		perror("start capturing is fail");
		exit(EXIT_FAILURE);
	}

	return 0;
}

int init_lcd()
{
	fbfd = open("/dev/fb0",O_RDWR);
	if(fbfd < 0){
		perror("open lcd file is fail.");
		return 0;
	}

	printf("fbfd = %d \n",fbfd);

	ret = ioctl(fbfd,FBIOGET_VSCREENINFO,&varinfo);
	if(ret != 0){
		perror("get fbinfo var is fail.");
	}
	
	printf("xres: %d,yres: %d,bits_per_pixel: %d\n",varinfo.xres,varinfo.yres,varinfo.bits_per_pixel);
	
	screen_size = varinfo.xres * varinfo.yres * varinfo.bits_per_pixel / 8;
	
	printf("screen_size : %d \n",screen_size);

	virtual_addr = (char *)mmap(0, screen_size, PROT_READ|PROT_WRITE, MAP_SHARED,fbfd,0);
	if((int)virtual_addr == -1){
		perror("alloc virtual address is fail.");
		return 0;
	}
	printf("virtual_addr : %p\n",virtual_addr);	
	
}

int read_image(int fd)
{
	i = 0;
	read(fd,image_buf,LENGTH);
	for(y=0; y<480; y++)
	{
		for(x=0; x<640; x++)
		{
			long location = (y * varinfo.xres * 2) + x *2;
			new_rgb.RGB_h = image_buf[i];
			new_rgb.RGB_l = image_buf[i+1];
//			rgb = (new_rgb.RGB_h << 8) | new_rgb.RGB_l;
			rgb = (new_rgb.RGB_l << 8) | new_rgb.RGB_h;

			*((unsigned short *)(virtual_addr + location)) = rgb;
			i = i+2;
		}
	}
}

int stop_capturing(int fd)
{
	enum v4l2_buf_type type;

	//开始采集数据
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(fd,VIDIOC_STREAMOFF,&type) == -1){
		perror("stop capturing is fail");
		exit(EXIT_FAILURE);
	}
	
	munmap(virtual_addr, screen_size);
	close(fbfd);
	close(fd);	
	return 0;
}

int main(int argc, const char *argv[])
{
	int fd;
	fd = camera_device_open();	
	init_lcd();
	init_camera_attribute(fd);
	
	start_capturing(fd);

	while(1){
		read_image(fd);	
	}
	stop_capturing(fd);
	
	return 0;
}
