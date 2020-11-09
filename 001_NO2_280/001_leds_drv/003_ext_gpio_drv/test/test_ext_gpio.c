#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, const char *argv[])
{
	int fd,i = 0;
	int val = 1;
    
	fd = open("/dev/ext_gpio_drv", O_RDWR);

    if (fd < 0)
	{
		printf("can't open ext_gpio_drv!\n");
	}
    
	while (i < 10)
	{
		write(fd, &val, 4);
		sleep(1);
		val = !val;
        printf("val = %d\n",val);
		i++;
	}	

	close(fd);

	return 0;
}
