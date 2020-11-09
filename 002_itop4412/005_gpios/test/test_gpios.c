#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

#define GPIOS 32


int main(int argc , const char *argv[])
{
	int fd,i,cmd=2;
	char *leds = "/dev/skyfall_gpios";
	char *cmd0 = "0"; 
	char *cmd1 = "1";

	if(argc < 2)
	{
		printf("Usage %s <0 or 1>!\n",argv[0]);
		return -1;
	}
	
	printf("argv[0] is %s ===> argv[1] is %s!\n",argv[0],argv[1]);
	
	if(strcmp(argv[1], cmd0) == 0)
	{
		cmd = 0;
		printf("cmd is 0!\n");
	}
	if(strcmp(argv[1], cmd1) == 0)
	{
		cmd = 1;
		printf("cmd is 1!\n");
	}
	
	if((fd = open(leds,O_RDWR|O_NDELAY)) < 0)
	{
		printf("APP open %s is failed!\n",leds);
	}
	else
	{
		printf("APP open %s is success!\n",leds);
		
		for(i = 0; i < GPIOS; i++)
		{
			ioctl(fd,cmd,i);
			printf("APP ioctl %s ,cmd is %d,i is %d!\n",leds,cmd,i);
		}
	}
	
	close(fd);

return 0;
}