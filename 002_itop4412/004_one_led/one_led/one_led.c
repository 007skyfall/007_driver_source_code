#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main(int argc, const char * agrv[])
{
	int fd,i;
	char *led_node = "/dev/skyfall_007";
	

	if((fd = open(led_node,O_RDWR | O_NDELAY)) < 0)
	{
		printf("APP open %s failed!\n",led_node);
	}
	else
	{
		printf("APP open %s success!\n",led_node);
		for(i = 0; i< 10; ++i)
		{
			ioctl(fd,1,1);
			sleep(3);

			ioctl(fd,0,1);
			sleep(3);
		}
	}
	
	close(fd);
return 0;

}