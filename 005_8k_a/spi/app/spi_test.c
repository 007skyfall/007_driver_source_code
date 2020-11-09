#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <sys/ioctl.h>  
#include<sys/types.h>  
#include<sys/stat.h>  
#include<fcntl.h>     

int main(int argc, char **argv)  
{  
	int fd;  
	int i;
	char ch;
	int count=0;  
	char buf[]={0x11,0x22,0x33,0x44,0x55};  
	fd = open("/dev/s3c24xx_spi", O_RDWR);  
	if (fd < 0) {  
		perror("open device spi");  
		exit(-1);  
	}  
	for (i = 0; i < sizeof(buf)/sizeof(buf[0]); i++) {		
		write(fd, &buf[i], 1);
		read(fd, &ch, 1);  
		printf("read byte is: 0x%02X\n",ch);  
	}
	getchar();
	close(fd);  
	return 0;  
} 
