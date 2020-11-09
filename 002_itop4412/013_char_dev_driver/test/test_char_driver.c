#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

int  main(int argc, const char * argv[])
{
	int fd;
	char *skyfall_node0 = "/dev/char_dev_node0";
	char *skyfall_node1 = "/dev/char_dev_node1";


	if((fd = open(skyfall_node0,O_RDWR|O_NDELAY)) < 0)
	{
		printf("%s,%d\n",__func__,__LINE__);

		printf("APP open %s failed!\n",skyfall_node0);
	}
	else
	{
		printf("APP open %s success!\n",skyfall_node0);
	}
	
	close(fd);
	
	if((fd = open(skyfall_node1,O_RDWR|O_NDELAY)) < 0)
	{
		printf("APP open %s failed!\n",skyfall_node1);
	}
	else
	{
		printf("%s,%d\n",__func__,__LINE__);

		printf("APP open %s success!\n",skyfall_node1);
	}
	
	close(fd);

return 0;	
}
