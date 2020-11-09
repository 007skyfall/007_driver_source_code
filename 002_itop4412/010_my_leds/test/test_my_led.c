#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main(int argc, char * agrv[])
{
	int fd;
	char *hello_node = "/dev/led3_demo";
	
	if((fd = open(hello_node,O_RDWR|O_NDELAY))<0)
	{
		printf("APP open %s failed\n",hello_node);
	}
	else
	{
		printf("APP open %s success\n",hello_node);
	}

close(fd);

return 0;

}