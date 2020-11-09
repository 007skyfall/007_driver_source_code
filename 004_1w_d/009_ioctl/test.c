#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include "myioctl.h"

char buf[128] = "i am user.............";

int main(int argc, const char *argv[])
{
	int fd;
	fd = open("/dev/my_ioctl",O_RDWR);
	if(fd == -1)
	{
		perror("open error");
		return -1;
	}

	ioctl(fd,ACCESS_INT,100);
	ioctl(fd,ACCESS_STR_W,buf);
	memset(buf,0,sizeof(buf));
	ioctl(fd,ACCESS_STR_R,buf);
	printf("udata = %s\n",buf);
	
	close(fd);

	return 0;
}
