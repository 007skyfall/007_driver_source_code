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
typedef long LONG;  
typedef unsigned long DWORD;  
typedef unsigned short WORD;  
  
typedef struct {  
        WORD    bfType;  
        DWORD   bfSize;  
        WORD    bfReserved1;  
        WORD    bfReserved2;  
        DWORD   bfOffBits;  
} BMPFILEHEADER_T;  
  
typedef struct{  
        DWORD      biSize;  
        LONG       biWidth;  
        LONG       biHeight;  
        WORD       biPlanes;  
        WORD       biBitCount;  
        DWORD      biCompression;  
        DWORD      biSizeImage;  
        LONG       biXPelsPerMeter;  
        LONG       biYPelsPerMeter;  
        DWORD      biClrUsed;  
        DWORD      biClrImportant;  
} BMPINFOHEADER_T;

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

#if 0
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
#endif

void savebmp(char * pdata, char * bmp_file, int width, int height )  
{      //分别为rgb数据，要保存的bmp文件名，图片长宽  
       int size = width*height*2*sizeof(char); // 每个像素点3个字节  
       // 位图第一部分，文件信息  
       BMPFILEHEADER_T bfh;  
       bfh.bfType = (WORD)0x4d42;  //bm  
       bfh.bfSize = size  // data size  
              + sizeof( BMPFILEHEADER_T ) // first section size  
              + sizeof( BMPINFOHEADER_T ) // second section size  
              ;  
       bfh.bfReserved1 = 0; // reserved  
       bfh.bfReserved2 = 0; // reserved  
       bfh.bfOffBits = sizeof( BMPFILEHEADER_T )+ sizeof( BMPINFOHEADER_T );//真正的数据的位置  
  
       // 位图第二部分，数据信息  
       BMPINFOHEADER_T bih;  
       bih.biSize = sizeof(BMPINFOHEADER_T);  
       bih.biWidth = width;  
       bih.biHeight = -height;//BMP图片从最后一个点开始扫描，显示时图片是倒着的，所以用-height，这样图片就正了  
       bih.biPlanes = 1;//为1，不用改  
       bih.biBitCount = 16;  
       bih.biCompression = 0;//不压缩  
       bih.biSizeImage = size;  
       bih.biXPelsPerMeter = 2835 ;//像素每米  
       bih.biYPelsPerMeter = 2835 ;  
       bih.biClrUsed = 0;//已用过的颜色，24位的为0  
       bih.biClrImportant = 0;//每个像素都重要  
       FILE * fp = fopen( bmp_file,"wb" );  
       if( !fp ) return;  
  
       fwrite( &bfh, 8, 1,  fp );//由于linux上4字节对齐，而信息头大小为54字节，第一部分14字节，第二部分40字节，所以会将第一部分补齐为16自己，直接用sizeof，打开图片时就会遇到premature end-of-file encountered错误  
       fwrite(&bfh.bfReserved2, sizeof(bfh.bfReserved2), 1, fp);  
       fwrite(&bfh.bfOffBits, sizeof(bfh.bfOffBits), 1, fp);  
       fwrite( &bih, sizeof(BMPINFOHEADER_T),1,fp );  
       fwrite(pdata,size,1,fp);  
       fclose( fp );  
}  
int read_image(int fd)
{
	read(fd,image_buf,LENGTH);
	savebmp(image_buf,"picture.bmp",640,480);
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
