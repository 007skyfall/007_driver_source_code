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
#include <pthread.h>

#define X_SIZE 640
#define Y_SIZE 480
#define LENGTH 614400

struct RGB_COMB {
	unsigned char RGB_h;
	unsigned char RGB_l;
};

int fb_fd;
int cam_fd;
int msgqid;
int ret;
int x,y;
unsigned int screen_size;
char * virtual_addr;
char * image_buf;
struct fb_var_screeninfo varinfo;
struct RGB_COMB new_rgb;
unsigned short rgb;

void camera_device_open(void)
{
	//用阻塞模式打开摄像头设备
	cam_fd = open("/dev/video0",O_RDWR,0);
	if(cam_fd < 0){
		perror("open /dev/video0 is fail.\n");
		exit(EXIT_FAILURE);
	}
}

int init_camera_attribute(void)
{
	int numBufs;
	v4l2_std_id id;
	struct v4l2_format fmt;
	struct v4l2_requestbuffers  req;
	struct v4l2_buffer    buf;

	//检查当前视频设备支持的标准
	ioctl(cam_fd,VIDIOC_QUERYSTD,&id);
	
	//设置视频捕获格式
	memset(&fmt,0,sizeof(fmt));
	fmt.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = 0;
	fmt.fmt.pix.height = 0;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;

	if(ioctl(cam_fd,VIDIOC_S_FMT,&fmt) == -1){
		perror("set VIDIOC_S_FMT is fail");
		exit(EXIT_FAILURE);
	}
	
	//分配内存
	memset(&req,0,sizeof(req));
	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	
	if(ioctl(cam_fd,VIDIOC_REQBUFS,&req) == -1){
		perror("set VIDIOC_REQBUFS is fail");
		exit(EXIT_FAILURE);
	}

	return 0;
}

int start_capturing(void)
{
	enum v4l2_buf_type type;

	//开始采集数据
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(cam_fd,VIDIOC_STREAMON,&type) == -1){
		perror("start capturing is fail");
		exit(EXIT_FAILURE);
	}

	return 0;
}

int init_lcd(void)
{
	fb_fd = open("/dev/fb0",O_RDWR);
	if(fb_fd < 0){
		perror("open lcd file is fail.");
		return 0;
	}

	printf("fb_fd = %d \n",fb_fd);

	ret = ioctl(fb_fd,FBIOGET_VSCREENINFO,&varinfo);
	if(ret != 0){
		perror("get fbinfo var is fail.");
	}
	
	printf("xres: %d,yres: %d,bits_per_pixel: %d\n",varinfo.xres,varinfo.yres,varinfo.bits_per_pixel);
	
	screen_size = varinfo.xres * varinfo.yres * varinfo.bits_per_pixel / 8;
	
	printf("screen_size : %d \n",screen_size);

	virtual_addr = (char *)mmap(0, screen_size, PROT_READ|PROT_WRITE, MAP_SHARED,fb_fd,0);
	if((int)virtual_addr == -1){
		perror("alloc virtual address is fail.");
		return 0;
	}
	printf("virtual_addr : %p\n",virtual_addr);	
	
}

int read_image(void)
{
	int i,ret;
	for(i=0; i<4; i++){
		image_buf = (char *)malloc(LENGTH);
		if(image_buf == NULL){
			perror("alloc memory is fail.\n");
			return -1;
		}
		read(cam_fd,image_buf,LENGTH);
	}
}

int write_image(void)
{
	int i = 0,ret;
	for(y=0; y<Y_SIZE; y++)
	{
		for(x=0; x<X_SIZE; x++)
		{
			long location = (y * varinfo.xres * 2) + x *2;
			new_rgb.RGB_h = wmsg.mtext[i];
			new_rgb.RGB_l = wmsg.mtext[i+1];
			rgb = (new_rgb.RGB_l << 8) | new_rgb.RGB_h;

			*((unsigned short *)(virtual_addr + location)) = rgb;
			i = i+2;
		}
	}
}

int stop_capturing(void)
{
	enum v4l2_buf_type type;

	//开始采集数据
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(cam_fd,VIDIOC_STREAMOFF,&type) == -1){
		perror("stop capturing is fail");
		exit(EXIT_FAILURE);
	}
	
	munmap(virtual_addr, screen_size);
	close(fb_fd);
	close(cam_fd);	
	return 0;
}

void * read_thread(void *arg)
{
	while(1){
		read_image();	
	}
}

void * write_thread(void *arg)
{
	while(1){
		write_image();
	}
}

int main(int argc, const char *argv[])
{
	pthread_t r_tid,w_tid;
	key_t key;
	camera_device_open();	
	init_lcd();
	init_camera_attribute();
	start_capturing();

	pthread_create(&r_tid,NULL,read_thread,NULL);
	pthread_create(&w_tid,NULL,write_thread,NULL);
	pthread_join(r_tid,NULL);
	pthread_join(w_tid,NULL);
	stop_capturing();
	
	return 0;
}
