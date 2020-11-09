#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#if 0
	操作说明：
	1.	argv[1] is cmd , argv[2] is io_arg;
	2.	./a.out 0 0  -----> LED2 off
	3.	./a.out 1 0  -----> LED2 on
	4.	./a.out 0 1  -----> LED3 off
	5.	./a.out 1 1  -----> LED3 on
	6.	./a.out 0 0  -----> LED2 off

#endif
/*argv[1] is cmd , argv[2] is io_arg*/
int main(int argc , char **argv)
{
	int fd,i = 0;
	char *lednode = "/dev/chardevnode0";

	/*O_RDWR读写方式,O_NDELAY非阻塞方式*/	
	if((fd = open(lednode,O_RDWR|O_NDELAY))<0)
	{
		printf("APP open %s failed!\n",lednode);
	}
	else
	{
		printf("APP open %s success!\n",lednode);
		ioctl(fd,atoi(argv[1]),atoi(argv[2]));
		printf("APP ioctl %s ,cmd is %s! io_arg is %s!\n",lednode,argv[1],argv[2]);
	}
	while (i < 10)
	{
		//LED2 LED3 all on
		ioctl(fd,1,0);
		ioctl(fd,1,1);
		sleep(3);
		//LED2 LED3 all off
		ioctl(fd,0,0);
		ioctl(fd,0,1);
		sleep(3);
		//LED2 on LED3 off
		ioctl(fd,1,0);
		ioctl(fd,0,1);
		sleep(3);
		//LED2 off LED3 on
		ioctl(fd,0,0);
		ioctl(fd,1,1);
		sleep(3);
		++i;
	}
	
	
	close(fd);

return 0;
}