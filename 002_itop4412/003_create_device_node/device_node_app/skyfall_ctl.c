#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main(int argc, const char *argv[])
{
	int fd;

	char *skyfall_node = "/dev/skyfall_007";
	
	if((fd = open(skyfall_node,O_RDWR|O_NDELAY)) < 0)
	{
		printf("APP open %s failed\n",skyfall_node);
	}

	else
	{
		printf("APP open %s success\n",skyfall_node);

		ioctl(fd,1,5);
	}
	
	close(fd);
	
	return 0;
}

