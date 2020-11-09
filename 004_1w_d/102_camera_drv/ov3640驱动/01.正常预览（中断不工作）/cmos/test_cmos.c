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

#define LENGTH 614400
char image_buf[LENGTH] = {0};

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

int build_picture(void *addr)
{
	FILE *fp;
	static int num=0;
	char picture_name[20];
	sprintf(picture_name,"picture%d.jpg",num);
	
	fp = fopen(picture_name,"w");
	if(fp == NULL){
		perror("fail to open ");
		exit(EXIT_FAILURE);
	}

	fwrite(addr,1,LENGTH,fp);

	fclose(fp);

	return 0;
}

int read_image(int fd)
{
	read(fd,image_buf,LENGTH);
	build_picture(image_buf);
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

	return 0;
}

int main(int argc, const char *argv[])
{
	int fd;
	fd = camera_device_open();	
	
	init_camera_attribute(fd);
	
	start_capturing(fd);
	
	read_image(fd);

	stop_capturing(fd);
	
	return 0;
}
