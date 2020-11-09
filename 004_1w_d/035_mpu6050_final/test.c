#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

char buf[6] = {0};
short x,y,z;

int main(int argc, const char *argv[])
{
	int fd;
	fd = open("/dev/mpu6050",O_RDWR);
	if(fd == -1){
		perror("open /dev/mpu6050 error");
		return -1;
	}

	while(1){
		read(fd,buf,sizeof(buf));
		x = buf[0];
		x = x<<8;
		x |= buf[1];

		y = buf[2];
		y = y<<8;
		y |= buf[3];

		z = buf[4];
		z = z<<8;
		z |= buf[5];
		
		printf("accel:x:%d,y:%d,z:%d\n",x,y,z);
		sleep(1);
	}
	
	close(fd);

	return 0;
}

